from flask import Flask
from flask import send_from_directory
from flask import request
import os

url_override = os.environ.get('BASE_URL')
if (url_override is not None and not url_override.endswith('/')):
    url_override = url_override + '/'

app = Flask(__name__)
global version
with open("/.version", "r") as f:
    version = f.readline().rstrip()

@app.route("/version.json")
def version_json():
    global version
    base_url = ""
    
    if url_override is None:
        base_url = request.host_url
    else:
        base_url = url_override

    return {
        "type": "home-sensor",
        "version": version,
        "url": base_url + "fw/firmware-" + version + ".bin"
    }

@app.route('/fw/<path:path>')
def firmware(path):
    return send_from_directory('/assets', path)

app.run(host='0.0.0.0', port=80)