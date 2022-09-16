#!/usr/bin/env python3 

import json, sys;

def main():

    with open("./vaktija.json", "r") as inputFile:
        inputJson = json.load(inputFile)
        
        diffCounter = 0
        montCounter = 0


        for difference in inputJson["differences"]:
            print("{ /*", diffCounter, "*/")
            
            for month in difference["months"]:

                for idx in range(6):
                    
                    if idx != 5:
                        if idx == 0:
                            print("    {", month["vakat"][idx], end=", ")
                        else:
                            print(month["vakat"][idx], ",", end=" ")
                    else:
                        if montCounter == 11:
                            print(month["vakat"][idx], " }")
                        else:
                            print(month["vakat"][idx], " },")
                print()
                
                montCounter += 1
                # print(month)
            
            if diffCounter == 117:
                print("}")
            else:
                print("},")
            
            diffCounter += 1
            montCounter = 0


if "__main__":
    main()