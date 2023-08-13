#ifndef __DEVICE_TREE_HPP_
#define __DEVICE_TREE_HPP_

#ifdef __arm64__

namespace xnu
{
    class Kernel;
    class DeviceTree;

    #define DT_KEY_LEN 0x20

    struct DeviceTreeNode
    {
        uint32_t n_properties;
        uint32_t n_children;
    };

    struct DeviceTreeProperty
    {
        char name[DT_KEY_LEN];
        uint32_t size;

        char val[0];
    };

    typedef bool (^dt_node_callback_t) (uint32_t depth, void *node, uint32_t size);
    typedef bool (^dt_property_callback_t) (uint32_t depth, void *prop, uint32_t size);

    struct PE_Video
    {
        unsigned long   v_baseAddr;     /* Base address of video memory */
        unsigned long   v_rowBytes;     /* Number of bytes per pixel row */
        unsigned long   v_width;        /* Width */
        unsigned long   v_height;       /* Height */
        unsigned long   v_depth;        /* Pixel Depth */
        unsigned long   v_display;      /* Text or Graphics */
        char            v_pixelFormat[64];
        unsigned long   v_offset;       /* offset into video memory to start at */
        unsigned long   v_length;       /* length of video memory (0 for v_rowBytes * v_height) */
        unsigned char   v_rotate;       /* Rotation: 0:normal, 1:right 90, 2:left 180, 3:left 90 */
        unsigned char   v_scale;        /* Scale Factor for both X & Y */
        char            reserved1[2];
    #ifdef __LP64__
        long            reserved2;
    #else
        long            v_baseAddrHigh;
    #endif
    };

    typedef struct PE_Video       PE_Video;

    #define kPEGraphicsMode         1
    #define kPETextMode             2
    #define kPETextScreen           3
    #define kPEAcquireScreen        4
    #define kPEReleaseScreen        5
    #define kPEEnableScreen         6
    #define kPEDisableScreen        7
    #define kPEBaseAddressChange    8
    #define kPERefreshBootGraphics  9


    typedef struct PE_state
    {
        boolean_t        initialized;
        PE_Video         video;
        void            *deviceTreeHead;
        void            *bootArgs;
        size_t           deviceTreeSize;
    } PE_state_t;

    static DeviceTree *deviceTree;
    
    PE_state_t* PE_state(Kernel *kernel);
    
    void*      getDeviceTreeHead(Kernel *kernel);
    size_t     getDeviceTreeSize(Kernel *kernel);

    class DeviceTree
    {
        public:
            explicit DeviceTree(Kernel *kernel) kernel(kernel) : device_tree(xnu::getDeviceTreeHead(kernel)) : device_tree_sz(xnu::getDeviceTreeSize(kernel)) { }

            void* getDeviceTree() { return reinterpret_cast<void*>(device_tree); }
            
            size_t getDeviceTreeSize() { return device_tree_sz; }

            size_t getNodeSize(uint32_t depth, DeviceTreeNode *node);

            DeviceTreeNode* findNode(char *nodename, uint32_t *depth);
            DeviceTreeProperty* findProperty(char *nodename, char *propname);

            void printData(uint8_t *prop_data, uint32_t prop_size);
            void printNode(char *nodename);

            void printDeviceTree();

        private:
            Kernel *kernel;

            mach_vm_address_t device_tree;

            size_t device_tree_sz;
    };

};

#endif

#endif