import os
import re

finname = 'HeatFragment.h'
foutname = 'HeatFragment.tmp'
fin = open(finname,'r')
fout = open(foutname,'w')


print('Processing HeatFragment (simplify FileIDs)')
for line in fin:
        parsedLine = re.findall(r'\bCALLIGRA_VERSION_STRING "([A-Za-z0-9. ]*)"', line) 
        
        if parsedLine != []:
                break
		fout.write(line);
    
fout.close()
print('Complete')