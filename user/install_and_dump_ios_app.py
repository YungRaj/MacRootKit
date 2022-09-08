# /usr/bin/python3

import sys, os, re, subprocess, plistlib, frida

def print_usage():
	print('python3 install_ios_app.py app_ids')

argv = sys.argv;
argc = len(sys.argv)

if(argc < 2):
	print_usage();

for i in range(1, argc):
	app_id = argv[i]

	command = ['sshpass', '-p', 'ilhan123', 'ssh', '-o', 'StrictHostKeyChecking=no', '-p', '2222', 'root@localhost', '/usr/bin/app_store_client', app_id]

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	print(output)

	output = output.decode('utf-8');

	if(len(output) == 0):
		continue

	plist_filename = output.split('.plist')[0] + '.plist'

	command = ['sshpass', '-p', 'ilhan123', 'scp', '-P', '2222', 'root@localhost:%s' % (plist_filename), './']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	plist_filename = plist_filename.split('/')[-1]

	print('plist = %s' % (plist_filename))

	plist = open(plist_filename, 'rb')

	plist_data = plistlib.load(plist);

	bundle_id = plist_data['bundleID']
	bundle_path = plist_data['bundlePath']
	store_item_id = plist_data['storeItemID']
	store_front = plist_data['storeFront']
	localized_name = plist_data['localizedName']

	if bundle_id == None:
		continue

	command = ['bagbak','--override', bundle_id]

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	decrypt_folder = './dump/' + bundle_id

	payload_folder = decrypt_folder + '/Payload'

	new_ipa = decrypt_folder + '/' + localized_name + '.ipa'

	command = ['zip', new_ipa, payload_folder, '-x', '\"*.DS_Store\"']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	app_bundle_name = bundle_path.split('/')[-1].split('.app')[0]

	app_bundle = payload_folder + '/' + app_bundle_name + ".app"

	entitlements = decrypt_folder + "/entitlements.xml"

	entitlements_file = open(entitlements, 'w+')

	command = ['codesign', '-d', '--entitlements', ':-', app_bundle]

	process = subprocess.Popen(command, stdout=entitlements_file, stderr=None)

	output, error = process.communicate()

	entitlements_file.close()

	command = ['codesign', '-fs', '-', '--entitlements', entitlements, app_bundle, '--deep']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	print(output)

	print('localizedName = %s' %(localized_name))