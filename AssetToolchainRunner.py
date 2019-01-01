from os import listdir
from os.path import isfile, join
import os.path
import subprocess
import json
import zlib
import pathlib
def GetCRC32(file):
    return zip.GetCRC32(file)

def qm(s):
    return "\"" + s + "\""


COMPRESSONATOR_ROOT = os.environ.get('COMPRESSONATOR_ROOT')
compressonatorPath = COMPRESSONATOR_ROOT + 'bin\\cli\\CompressonatorCLI.exe'
compressonatorPath = qm(compressonatorPath) 

def CompressTexture(filename, path, argument):
    absolutePath = os.path.abspath(path)
    pathWithFileName =  absolutePath + "\\" + filename + ".png "

    outputPath = absolutePath.replace(filename, "") + "\\" + filename.replace("png", "") + ".dds"
    
    print(filename + " to DDS " + argument)
    # checks if the destination is already existing
    outputFile = pathlib.Path(outputPath)
    if outputFile.is_file(): 
        return

    fullArgument = " -silent -miplevels 99 -fd " + argument + " " + pathWithFileName + outputPath
    # print(fullArgument)
    # subprocess.call([compressonatorPath, fullArgument], shell = True)
    os.system(compressonatorPath + fullArgument)
    


assetRootDir = "./Content/textures_pbr/"
# onlyfiles = [f for f in listdir(assetRootDir) if isfile(join(assetRootDir, f))]
for f in listdir(assetRootDir):
    pathSplited = (os.path.splitext(f))
    if(pathSplited[-1].lower() == ".png"):
        fileNameLower = pathSplited[0].lower()
        
        argument = ""
        if("albedo" in fileNameLower or "diffuse" in fileNameLower):
            # print(f + " is an albedo texture")
            CompressTexture(pathSplited[0], assetRootDir, "DXT3")
            pass
        if("normal" in fileNameLower):
            # print(f + " is an normal texture")
            CompressTexture(pathSplited[0], assetRootDir, "BC5")
            pass
        if("metallic" in fileNameLower):
            # print(f + " is an metallic texture")
            CompressTexture(pathSplited[0], assetRootDir, "BC4")
            pass
        if("roughness" in fileNameLower):
            # print(f + " is an roughness texture")
            # file_object = open(f, "r")
            CompressTexture(pathSplited[0], assetRootDir, "BC5")
    

print("DONE!")
