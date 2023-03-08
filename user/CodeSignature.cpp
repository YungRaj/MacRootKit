#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <CommonCrypto/CommonDigest.h>

#include "UserMachO.hpp"

using namespace mrk;

#define swap32(x) OSSwapInt32(x)

#define min(a, b) ((a) < (b) ? (a) : (b))

CodeSignature* CodeSignature::codeSignatureWithLinkedit(UserMachO *macho, struct linkedit_data_command *cmd)
{
	return new CodeSignature(macho, cmd);
}

bool CodeSignature::verifyCodeSlot(uint8_t *blob, size_t size, bool sha256, char *signature, size_t signature_size)
{
	bool verified = false;

	if(sha256)
	{
		unsigned char result[CC_SHA256_DIGEST_LENGTH];

		CC_SHA256(blob, static_cast<uint32_t> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA256_DIGEST_LENGTH, signature_size)));
	} else
	{
		unsigned char result[CC_SHA1_DIGEST_LENGTH];

		CC_SHA1(blob, static_cast<uint32_t> (size), result);

		verified = (memcmp(result, signature, min(CC_SHA1_DIGEST_LENGTH, signature_size)));
	}

	return verified;
}
	
bool CodeSignature::compareHash(uint8_t *hash1, uint8_t *hash2, size_t hashSize)
{
	const char *h1 = reinterpret_cast<const char*>(hash1);
	const char *h2 = reinterpret_cast<const char*>(hash2);

	return (memcmp(h1, h2, hashSize) == 0);
}

uint8_t* CodeSignature::computeHash(bool sha256, uint8_t *blob, size_t size)
{
	uint8_t *result;

	if(sha256)
	{
		result = static_cast<uint8_t*>(malloc(CC_SHA256_DIGEST_LENGTH));

		CC_SHA256(blob, static_cast<uint32_t> (size), result);
	} else
	{
		result =  static_cast<uint8_t*>(malloc(CC_SHA1_DIGEST_LENGTH));

		CC_SHA1(blob, static_cast<uint32_t> (size), result);
	}

	return result;
}


bool CodeSignature::parseCodeSignature()
{
	SuperBlob *superblob;

	uint32_t blobcount = swap32(superblob->count);

	uint32_t offset = this->cmd->dataoff;
	uint32_t size = this->cmd->datasize;
    
    MAC_RK_LOG("\t%u blobs\n",blobcount);
    
    for(uint32_t blobidx = 0; blobidx < blobcount; blobidx++)
    {
        BlobIndex index = superblob->index[blobidx];
        
        uint32_t blobtype = swap32(index.type);
        uint32_t bloboffset = swap32(index.offset);
        uint32_t begin = offset + bloboffset;
        
        Blob *blob = (Blob*) macho->getOffset(begin);
        
        uint32_t magic = swap32(blob->magic);
        uint32_t length = swap32(blob->length);
        
        switch(magic)
        {
            case CSMAGIC_CODEDIRECTORY:
                {
	                code_directory_t directory = (code_directory_t) macho->getOffset(begin);
	                
	                uint32_t hashOffset = swap32(directory->hashOffset);
	                uint32_t identOffset = swap32(directory->identOffset);
	                uint32_t nSpecialSlots = swap32(directory->nSpecialSlots);
	                uint32_t nCodeSlots = swap32(directory->nCodeSlots);
	                uint32_t hashSize = directory->hashSize;
	                uint32_t hashType = directory->hashType;
	                uint32_t pageSize = directory->pageSize;
	                
	                bool sha256 = false;
	                
	                char *ident = reinterpret_cast<char*>(macho->getOffset(begin + identOffset));
	                
	                MAC_RK_LOG("\tIdentifier: %s\n",ident);
	                MAC_RK_LOG("\tPage size: %u bytes\n",1 << pageSize);
	                
	                if(hashType == HASH_TYPE_SHA1)
	                {
	                    MAC_RK_LOG("\tCD signatures are signed with SHA1\n");
	                } else if(hashType == HASH_TYPE_SHA256)
	                {
	                	MAC_RK_LOG("\tCD signatures are signed with SHA256\n");

	                    sha256 = true;
	                } else 
	                {
	                    MAC_RK_LOG("\tUnknown hashing algorithm in pages\n");
	                }
	                
	                for(int i = 0; i < nCodeSlots; i++)
	                {
	                    uint32_t pages = nCodeSlots;
	                    
	                    if(pages)
	                        MAC_RK_LOG("\t\tPage %2u ",i);
	                    
	                    uint8_t *hash = (uint8_t*) macho->getOffset(begin + hashOffset + i * hashSize);
	                    
	                    for(int j = 0; j < hashSize; j++)
	                        MAC_RK_LOG("%.2x",hash[j]);

	                    uint8_t *blob = (uint8_t*) macho->getOffset(i * (1 << pageSize));
	                    
	                    if(i + 1 != nCodeSlots)
	                    {
	                        if(this->verifyCodeSlot(blob, 1 << pageSize, sha256,(char*)hash, hashSize))
	                            MAC_RK_LOG(" OK...");
	                        else
	                            MAC_RK_LOG(" Invalid!!!");
	                    } else
	                    {

	                        if(this->verifyCodeSlot(blob, 1 << pageSize, sha256,(char*)hash, hashSize))
	                        // hash the last page only until the code signature,
	                        // so that that code signature doesn't get included into hash
	                            MAC_RK_LOG(" OK...");
	                    }
	                    
	                    MAC_RK_LOG("\n");
	                }
	                
	                begin = offset + bloboffset - hashSize * nSpecialSlots;
	                
	                if(nSpecialSlots)
	                    MAC_RK_LOG("\n\tSpecial Slots\n");
	                
	                for(int i = 0; i < nSpecialSlots; i++)
	                {
	                    uint8_t *hash = (uint8_t*) macho->getOffset(begin + hashOffset + i * hashSize);

	                    if(i < 5)
	                        MAC_RK_LOG("\t\t%s ", special_slots[i].name);
	                    else
	                        MAC_RK_LOG("\t\t");
	                    
	                    for(int j = 0; j < hashSize; j++)
	                        MAC_RK_LOG("%.2x",hash[j]);
	                    
	                    
	                    special_slots[i].sha256 = (hashType == HASH_TYPE_SHA256);
	                    special_slots[i].hash = hash;
	                    special_slots[i].hashSize = hashSize;
	                    
	                    uint8_t *zerobuf = new uint8_t[hashSize];

	                    memset(zerobuf, 0x0, hashSize);
	                    
	                    if(memcmp(hash, zerobuf, hashSize) != 0)
	                    {
	                        if(i == BOUND_INFO_PLIST)
	                        {
	                            char *path = macho->getFilePath();
	                            
	                            char **res = NULL;
	                            bool found = false;
	                            
	                            uint32_t num_tokens = 0;
	                            uint32_t new_length = 0;
	                            
	                            char *app_dir = strtok(path, "/");
	                            
	                            while (app_dir)
	                            {
	                                new_length += strlen(app_dir);
	                                
	                                res = reinterpret_cast<char**>(realloc(res, sizeof (char*) * ++num_tokens));
	                                
	                                if (res == NULL)
	                                    break;
	                                
	                                res[num_tokens - 1] = app_dir;
	                                
	                                if(strcmp(app_dir,"MacOS") == 0)
	                                {
	                                    found = true;

	                                    break;
	                                }
	                                
	                                app_dir = strtok (NULL, "/");
	                            }
	                            
	                            if(!found)
	                                continue;
	                            
	                            new_length += num_tokens + 1;
	                            
	                            char *info_plist = new char[sizeof(char) * new_length];
	                            
	                            for(int j=0; j < num_tokens; j++)
	                            {
	                                struct stat s;

	                                strcat(info_plist,"/");
	                                strcat(info_plist,res[j]);

	                                char *possible_path = strdup(info_plist);
	                                
	                                if(stat(path,&s) == 0)
	                                {
	                                    if(s.st_mode & S_IFDIR)
	                                    {
	                                        FILE *fp;

	                                        strcat(possible_path, "/Info.plist");

	                                        fp = fopen(possible_path, "r");

	                                        if(fp)
	                                        {
	                                            fclose(fp);
	                                            free(possible_path);

	                                            break;
	                                        }
	                                    }
	                                }

	                                free(possible_path);
	                            }
	                            
	                            strcat(info_plist,"/Info.plist");
	                            
	                            FILE *info = fopen(info_plist, "r");

	                            if(info)
	                            {
	                            	fseek(info, 0, SEEK_END);
		                            size_t info_size = ftell(info);
		                            fseek(info, 0, SEEK_SET);
		                            
		                            uint8_t *info_buf = new uint8_t[info_size];
		                            fseek(info, 0, SEEK_SET);
		                            fread(info_buf, 1, size,info);
		                            
		                            fclose(info);
		                            
		                            uint8_t *info_hash = this->computeHash(special_slots[i].sha256, info_buf, (uint32_t)info_size);
		                            
		                            if(memcmp(info_hash, special_slots[i].hash, special_slots[i].hashSize) == 0)
		                                MAC_RK_LOG(" OK...");
		                            else
		                                MAC_RK_LOG(" Invalid!!!");
	                            }
	                        
	                        }
	                                                      
	                    }

	                    free(zerobuf);
	                                                  
	                    MAC_RK_LOG("\n");
	                }
	            }

                break;
            case CSMAGIC_BLOBWRAPPER:
                break;
            case CSMAGIC_REQUIREMENTS:
                break;
            case CSMAGIC_EMBEDDED_ENTITLEMENTS:
                {
	                uint8_t *blob_raw;
	                uint8_t *blob_hash;
	                
	                char *entitlements;
	                
	                entitlements = new char[length - sizeof(struct Blob)];
	                
	                memcpy(entitlements, macho->getOffset(begin + sizeof(struct Blob)), length - sizeof(struct Blob));
	                
	                blob_raw = (uint8_t*) macho->getOffset(begin);
	                blob_hash = this->computeHash(special_slots[ENTITLEMENTS].sha256, blob_raw, length);
	                
	                MAC_RK_LOG("\nEntitlements ");
	                
	                if(this->compareHash(special_slots[ENTITLEMENTS].hash, blob_hash, special_slots[ENTITLEMENTS].hashSize))
	                    MAC_RK_LOG("OK...\n");
	                else
	                    MAC_RK_LOG("Invalid!!!\n");
	                
	                MAC_RK_LOG("%s\n",entitlements);
	                
	                free(entitlements);
	            }
                
                break;
            default:
                ;
                break;
        }
    }

	return true;
}
