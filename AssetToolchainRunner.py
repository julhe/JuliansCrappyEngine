from os import listdir
from os.path import isfile, join
import os.path
import subprocess

COMPRESSONATOR_ROOT = os.environ.get('COMPRESSONATOR_ROOT')
compressonatorPath = COMPRESSONATOR_ROOT + 'bin\\cli\\CompressonatorCLI.exe'
compressonatorPath = "\"" + compressonatorPath + "\"" 

def CompressTexture(filename, path, argument):
    absolutePath = os.path.abspath(path)
    pathWithFileName =  absolutePath + "\\" + filename + ".tga "

    outputPath = absolutePath.replace(filename, "") + "\\" + filename.replace("tga", "") + ".dds"
    fullArgument = " -fd " + argument + " " + pathWithFileName + outputPath

    #  + "\" \"" + outputPath + "\""
    # print(fullArgument)
    print( fullArgument)
    #subprocess.call([compressonatorPath, fullArgument], shell = True)
    os.system(compressonatorPath + fullArgument)



path = "./Content/textures_pbr/"
onlyfiles = [f for f in listdir(path) if isfile(join(path, f))]
for f in listdir(path):
    
    pathSplited = (os.path.splitext(f))
    if(pathSplited[-1].lower() == ".tga"):
        fileNameLower = pathSplited[0].lower()
        
        if("albedo" in fileNameLower):
            # print(f + " is an albedo texture")
            # CompressTexture(path, "BC7")
            pass
        if("normal" in fileNameLower):
            # print(f + " is an normal texture")
            # CompressTexture(path, "BC5")
            pass
        if("metallic" in fileNameLower):
            # print(f + " is an metallic texture")
            # CompressTexture(path, "BC4")
            pass
        if("roughness" in fileNameLower):
            # print(f + " is an roughness texture")
            CompressTexture(pathSplited[0], path, "BC5")


call = compressonatorPath + ' -fd BC7 boat.png'
# print(call)
# subprocess.call(call, shell = True)
