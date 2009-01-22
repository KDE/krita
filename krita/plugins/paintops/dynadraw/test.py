import os
import sys

def renameFile(FileName,oldName,newName):
  # TODO 
#  CMD1 = "sed s/" + spray + "/"+ dyna + "/g ";
#  CMD2 = "sed s/" + spray + "/"+ dyna + "/g ";
#  CMD3 = "sed s/" + spray + "/"+ dyna + "/g ";

  CMD1 = "sed s/spray/dyna/g ";
  CMD2 = "sed s/Spray/Dyna/g ";
  CMD3 = "sed s/SPRAY/DYNA/g ";
  os.system(CMD1 + FileName + ">" + FileName);
  os.system(CMD2 + FileName + ">" + FileName);
  os.system(CMD3 + FileName + ">" + FileName);
  print FileName + " has been renamed."

def replaceFilenames():
  for fileName in sys.argv:
    if fileName != sys.argv[0]:
      print "Processing " + fileName + "...: ",
      renameFile(fileName, "spray", "dyna")
## TODO
def renameFilenames():
  for fileName in sys.argv:
    if fileName != sys.argv[0]:
      
      os.system("mv "+fileName+" "+newName)

def main():
  replaceFilenames()


main()