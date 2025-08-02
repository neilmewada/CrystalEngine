# This is a sample Python script.
import io
import pathlib
# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import sys
import argparse
from array import array
from pathlib import Path
import os
import shutil

def process_template(item, templateDict):
    item_name = item.name
    item = item.rename(item.parent.joinpath(item_name % templateDict))
    if os.path.isdir(item):
        for filename in os.listdir(item):
            process_template(item.joinpath(Path(filename)), templateDict)
    else:
        file = open(item, 'r')
        content = file.read()
        content = content % templateDict
        file.close()
        file = open(item, 'w')
        file.write(content)
        file.close()
    pass

def main():
    parser = argparse.ArgumentParser(description="Generate classes")
    parser.add_argument("tArgs", metavar="Arg", type=str, nargs="+",
                        help="Argument values to be given to the template generator")
    parser.add_argument('-c', '--class', dest='classTemplate', type=str,
                        help='Path to the local template to use', default="Object")
    parser.add_argument('-o', '--output', dest='outDir', type=str,
                        help='Output directory for generated content')
    args = parser.parse_args()
    classTemplatePath = Path(__file__).parent.absolute().joinpath('../Templates/ClassTemplates/' + args.classTemplate)
    outDir = Path(os.getcwd()).absolute().joinpath(args.outDir)
    if not classTemplatePath.exists():
        print('Error: Could not find template with at path: ' + classTemplatePath)
        return
    print('Class Template: ' + str(classTemplatePath))
    print('Output Dir: ' + str(outDir))

    namespaceFound = False
    superClassFound = False

    templateArgs = dict()
    for tArg in args.tArgs:
        if "=" in tArg:
            name = tArg.split("=")[0]
            templateArgs[name] = tArg.split("=")[1]
            templateArgs[name + "_Lower"] = tArg.split("=")[1].lower()
            templateArgs[name + "_Upper"] = tArg.split("=")[1].upper()
            if name == 'Module':
                templateArgs[name + "_API"] = tArg.split("=")[1].upper() + "_API"
            if name == "NameSpace":
                namespaceFound = True
            if name == "SuperClass":
                superClassFound = True
    
    if not namespaceFound:
        templateArgs["NameSpace"] = "CE"
        
    if not superClassFound:
        if templateArgs["NameSpace"].startswith("CE"):
            templateArgs["SuperClass"] = "Object"
        else:
            templateArgs["SuperClass"] = "CE::Object"
    
    print("Generating from template with following args:")
    print(templateArgs)

    topLevelFiles = []
    for filename in os.listdir(classTemplatePath):
        topLevelFiles.append(filename)
        src = classTemplatePath.joinpath(filename)
        dst = outDir.joinpath(filename)
        if os.path.isdir(src):
            shutil.copytree(src, dst)
            process_template(dst, templateArgs)
        else:
            shutil.copy(src, dst)
            process_template(dst, templateArgs)

    print("Successfuly generated from template at: " + str(outDir))
    return 0

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    main()

