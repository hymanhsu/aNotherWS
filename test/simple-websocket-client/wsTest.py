#!/usr/bin/env python
#coding=utf-8

#author: haisheng.yu
#history:
#   initial version, 2017.04.28

import os, sys, time, datetime
import json
import pprint
import websocket
from websocket import create_connection


print sys.getdefaultencoding()
reload(sys)
sys.setdefaultencoding('utf-8')
print sys.getdefaultencoding()

success = 0
#to set request times, if this number of request all failure, mark final result failure and send alarm email
REQUEST_TIME = 1



def uToChar(unicodeList):
    str = ""
    for u in unicodeList:
        str += u
    return str

def isSuccess(result, expectResult):
    global success
    try :
        print("actual result: %s\n"%result)
        expectResult = str(expectResult)
        if ( expectResult.replace("\"", "'") == result.replace("\"", "'") ):
            success += 1
            print("[SUCCESS], the data of sended and received all same\n")
        else:
            print("[FAIL] actual result NOT same as expect result: %s\n"%expectResult)
            
    except Exception as inst:
        print("Exception found in result check, please double check prcessing logs\nException: %s\n"%inst);

def printJson(jsonStr):
    print("query json:\n")
    print json.dumps(jsonStr, sort_keys=True, indent=2, separators=(',', ': '))

def onlineWebsocketCheck(uri="ws://127.0.0.1:8080"):
    print uri
    #websocket.enableTrace(True)
    ws = create_connection(uri)
    stampt = datetime.datetime.now().strftime("%Y_%m_%d_%H:%M:%S")
    sessionId = "test_webcliet_{}".format(stampt)
    userId = "test_webclient_user_{}".format(stampt)
    dic = {"access_token":"test-accessToken"}

    printJson(dic)
    for i in range(REQUEST_TIME) :
        ws.send(json.dumps(dic))
        result = ws.recv()
        isSuccess(result, dic)
        time.sleep(1)
    ws.close()



if "__main__" == __name__:
    onlineWebsocketCheck()
