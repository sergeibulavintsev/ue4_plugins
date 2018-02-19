import os
import subprocess
import regenerateforue4

grpc_dir=r"..\..\..\ThirdParty\grpc\bin\Win64"
protoc=os.path.join(grpc_dir, "protoc.exe")
plugin=os.path.join(grpc_dir, "grpc_cpp_plugin.exe")
curr_dir = os.getcwd()
for filename in os.listdir(curr_dir):
    if filename.endswith(".proto"):
        subprocess.call([protoc, "--grpc_out=.", "--plugin=protoc-gen-grpc={}".format(plugin), filename], shell=True)
        subprocess.call([protoc, "--cpp_out=dllexport_decl=MY_MACRO:.", filename])
		
for proto_generated in os.listdir(curr_dir):
    if proto_generated.endswith(".pb.cc"):
        print("fixing file: " + proto_generated)
        regenerateforue4.Generate(proto_generated)
