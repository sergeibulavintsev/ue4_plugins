import os
import sys

include_allow_header = '#include "AllowWindowsPlatformTypes.h"'
include_hide_header = '#include "HideWindowsPlatformTypes.h"'
common_warnings = """
#pragma warning(push)
#pragma warning(disable : 4668)
#pragma warning(disable : 4456)
#pragma warning(disable : 4125)
#pragma warning(disable : 4100)
#pragma warning(disable : 4800)
"""
grpc_warnings = common_warnings + """#pragma warning(disable : 4005)
#pragma warning(disable : 4291)
#pragma warning(disable : 4505)"""

def Check(_CodeFile):
    if (os.path.isfile(_CodeFile) is False):
        print "Can't find the code file!!!"
        return False

    code_file = open(_CodeFile, 'r')
    if (code_file is None):
        print "Failed to read the code file!!!"
        return False
    code_lines = code_file.readlines()
    code_file.close()

    for line in code_lines[:10]:
        if (line.find(include_allow_header) < 0 and line.find(include_hide_header) < 0):
            continue
        print "Already was regenerated?"
        return False
    return True

def Generate(_CodeFile):
    grpc_file = "grpc" in _CodeFile
    if (Check(_CodeFile) is False):
        print "Failed to check the code file!!!"
        return

    if (os.path.isfile(_CodeFile) is False):
        print "Can't find the code file!!!"
        return

    code_file = open(_CodeFile, 'r')
    if (code_file is None):
        print "Failed to read the code file!!!"
        return
    code_lines = code_file.readlines()
    code_file.close()

    code_file = open(_CodeFile, 'w')
    if (code_file is None):
        print "Failed to write the code file!!!"
        return
   
    pragma_inserted = False
    for line_num in range(len(code_lines)):
        line = code_lines[line_num]
        if not pragma_inserted and line.startswith("#include"):
            code_file.write('{0}\n\n'.format(grpc_warnings if grpc_file else common_warnings))
            pragma_inserted = True
        code_file.write(line)
    code_file.write("#pragma warning( pop )")
    code_file.close()
    print "Success to regenerate the code for UE4"

if __name__ == "__main__":
	if len(sys.argv) <= 1:
		print "Usage: python regenerateforue4.py `code file`"
	else:
            Generate(sys.argv[1])
