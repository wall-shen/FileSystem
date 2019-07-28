from flask import Flask, request, jsonify
import json;
import sys;
import os;

app = Flask(__name__)

DefaultPath = '/home/wall/data/'

@app.route('/download', methods=['GET', 'POST'])
def ReadFile():
    if request.method == 'GET':
        fileName = request.args.get('fileName')
        pos = int(request.args.get('pos'))
        length = int(request.args.get('length'))
        fileName = DefaultPath + fileName
        print(fileName)
        if os.path.exists(fileName):
            size = os.path.getsize(fileName)        

            if pos < 0 or length <= 0:
                result = json.jsonify(length = 0, data = '', size = size)
                print(result)
                return result
            fp = open(fileName, 'r')
            fp.seek(pos, 0)
            data = fp.read(length)
            print(data)
            length = len(data)
            
            result = jsonify(length = length, data = data, size = size)
            print(result)
            return result
        result = jsonify(length = 0, data = '', size = 0)
        return result

@app.route('/hello')
def hello():
    return 'hello'

if __name__ == '__main__':
    app.run(host = '0.0.0.0' , port = 8001)
        
    
