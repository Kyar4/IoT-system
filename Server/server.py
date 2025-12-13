from flask import Flask, request, Response, jsonify
import time
import threading
import numpy as np
import cv2

app = Flask(__name__)

lock = threading.Lock()
last_frame_raw = None
last_frame_annot = None
last_ts = 0.0
last_result = {"ok": 0, "face": 0, "count": 0, "ts": 0.0}

cascade_path = cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
face_cascade = cv2.CascadeClassifier(cascade_path)
if face_cascade.empty():
    raise RuntimeError(f"Cannot load Haar cascade: {cascade_path}")

def detect_faces_and_annotate(jpeg_bytes: bytes):
    img_np = np.frombuffer(jpeg_bytes, np.uint8)
    img = cv2.imdecode(img_np, cv2.IMREAD_COLOR)
    if img is None:
        return None, None, 0

    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray = cv2.equalizeHist(gray)

    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.2,
        minNeighbors=4,
        minSize=(40, 40)
    )

    count = 0 if faces is None else len(faces)

    if count > 0:
        for (x, y, w, h) in faces:
            cv2.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 2)

    ok, enc = cv2.imencode(".jpg", img, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
    if not ok:
        return img, None, int(count)

    return img, enc.tobytes(), int(count)

@app.route("/upload", methods=["POST"])
def upload():
    global last_frame_raw, last_frame_annot, last_ts, last_result

    data = request.get_data()
    if not data:
        return jsonify({"ok": 0, "err": "no_data"}), 400

    t = time.time()

    img, annot_jpeg, face_count = detect_faces_and_annotate(data)
    if img is None:
        return jsonify({"ok": 0, "err": "bad_jpeg"}), 400

    res = {
        "ok": 1,
        "face": 1 if face_count > 0 else 0,
        "count": face_count,
        "ts": t
    }

    with lock:
        last_frame_raw = data
        last_frame_annot = annot_jpeg if annot_jpeg is not None else data
        last_ts = t
        last_result = res

    print(f"Frame: {len(data)} bytes | face={res['face']} | count={face_count}")
    return jsonify(res), 200

@app.route("/latest", methods=["GET"])
def latest():
    with lock:
        if last_frame_raw is None:
            return "no frame yet", 404
        use_annot = request.args.get("annot", "1") == "1"
        frame = last_frame_annot if (use_annot and last_frame_annot is not None) else last_frame_raw

    return Response(frame, mimetype="image/jpeg")

@app.route("/status", methods=["GET"])
def status():
    with lock:
        return jsonify(last_result), 200

@app.route("/", methods=["GET"])
def index():
    return """
<html>
  <head>
    <title>ESP32 Camera + Face Detect</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { background:#000; color:#fff; text-align:center; font-family:Arial; margin:0; padding:0; }
      .wrap { padding: 14px; }
      img { max-width: 100%; height: auto; border: 2px solid #444; border-radius: 8px; }
      .row { display:flex; justify-content:center; gap:10px; flex-wrap:wrap; margin:10px 0; }
      .box { background:#111; border:1px solid #333; border-radius:10px; padding:10px 12px; min-width: 160px; }
      .ok { color:#00ff88; font-weight:700; }
      .no { color:#ff4477; font-weight:700; }
      small { color:#aaa; }
    </style>
  </head>
  <body>
    <div class="wrap">
      <h2>ESP32 Camera + Face Detect</h2>

      <div class="row">
        <div class="box">
          <div>FACE</div>
          <div id="face" class="no">--</div>
        </div>
        <div class="box">
          <div>COUNT</div>
          <div id="count">--</div>
        </div>
        <div class="box">
          <div>TS</div>
          <div><small id="ts">--</small></div>
        </div>
      </div>

      <img id="img" src="/latest?annot=1" />
      <p><small>Auto reload ~300ms</small></p>
    </div>
  </body>

  <script>
    function refresh() {
      const img = document.getElementById('img');
      img.src = '/latest?annot=1&ts=' + Date.now();

      fetch('/status?ts=' + Date.now())
        .then(r => r.json())
        .then(j => {
          const faceEl = document.getElementById('face');
          const countEl = document.getElementById('count');
          const tsEl = document.getElementById('ts');

          if (j && j.ok) {
            faceEl.textContent = j.face ? 'DETECTED' : 'NO';
            faceEl.className = j.face ? 'ok' : 'no';
            countEl.textContent = j.count;
            tsEl.textContent = j.ts;
          }
        })
        .catch(() => {});
    }

    setInterval(refresh, 300);
  </script>
</html>
"""

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000, debug=False, threaded=True)
