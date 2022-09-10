# /usr/bin/python3

import sys, os, re, subprocess, plistlib, frida

def print_usage():
	print('python3 install_ios_app.py app_ids')

argv = sys.argv;
argc = len(sys.argv)

if argc < 2:
	print_usage();

for i in range(1, argc):
	app_id = argv[i]

	command = ['sshpass', '-p', 'ilhan123', 'ssh', '-o', 'StrictHostKeyChecking=no', '-p', '2222', 'root@localhost', '/usr/bin/app_store_client', app_id]

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8');

	if len(output) == 0:
		continue
	if'Error' in output:
		continue

	plist_filename = output.split('.plist')[0] + '.plist'

	command = ['sshpass', '-p', 'ilhan123', 'scp', '-P', '2222', 'root@localhost:%s' % (plist_filename), './']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	plist_filename = plist_filename.split('/')[-1]

	plist = open(plist_filename, 'rb')

	plist_data = plistlib.load(plist);

	bundle_id = plist_data['bundleID']
	bundle_path = plist_data['bundlePath']
	store_item_id = plist_data['storeItemID']
	store_front = plist_data['storeFront']
	localized_name = plist_data['localizedName']

	if bundle_id == None:
		continue

	command = ['bagbak','--no-extension', '--override', bundle_id]

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	if error is not None:
		continue

	decrypt_folder = './dump/' + bundle_id

	payload_folder = decrypt_folder + '/Payload'

	new_ipa = decrypt_folder + '/' + localized_name + '.ipa'

	app_bundle_name = bundle_path.split('/')[-1].split('.app')[0]

	app_bundle = payload_folder + '/' + app_bundle_name + ".app"

	entitlements = decrypt_folder + "/entitlements.xml"

	entitlements_file = open(entitlements, 'w+')

	command = ['sshpass', '-p', 'ilhan123', 'ssh', '-o', 'StrictHostKeyChecking=no', '-p', '2222', 'root@localhost', 'ps aux']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	stream = output.split('\n')

	pid = None

	for line in stream:
		if bundle_path in line or bundle_path.split('/')[-1] in line:
			output = subprocess.check_output(['awk', '{print $2}'], input=line, text=True)

			pid = int(output)

	command = ['sshpass', '-p', 'ilhan123', 'ssh', '-o', 'StrictHostKeyChecking=no', '-p', '2222', 'root@localhost', 'kill', '-9', '%d' % (pid)]

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	command = ['codesign', '-d', '--entitlements', ':-', app_bundle]

	process = subprocess.Popen(command, stdout=entitlements_file, stderr=None)

	output, error = process.communicate()

	entitlements_file.close()

	command = ['codesign', '-fs', '-', '--entitlements', entitlements, app_bundle, '--deep']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	command = ['zip', '-r',  new_ipa, payload_folder, '-x', '\"*.DS_Store\"']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')

	command = ['sshpass', '-p', 'ilhan123', 'ssh', '-o', 'StrictHostKeyChecking=no', '-p', '2222', 'root@localhost', 'open', 'com.apple.AppStore']

	process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=None)

	output, error = process.communicate()

	output = output.decode('utf-8')