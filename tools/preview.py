"""Offline UI preview only. Never opens serial/BLE or proxies control requests."""
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path=='/':
            s=(root/'include/panel.h').read_text(encoding='utf-8').split('R"W96PAGE(',1)[1].rsplit(')W96PAGE";',1)[0]
            s=s.replace('__W96P_CSRF__','offline-preview').replace('C3 本地控制 · 米家桥接','离线预览 · 不连接设备')
            data=s.encode();ctype='text/html; charset=utf-8'
        elif self.path=='/api/status':
            data=json.dumps({'ip':'离线预览','ble':False,'cloud':False,'configured':False}).encode();ctype='application/json'
        else:
            self.send_error(404);return
        self.send_response(200);self.send_header('Content-Type',ctype);self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
    def do_POST(self):
        data=json.dumps({'error':'离线预览不操作设备'},ensure_ascii=False).encode()
        self.send_response(409);self.send_header('Content-Type','application/json');self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
    def log_message(self,*args):pass
HTTPServer(('127.0.0.1',8769),Handler).serve_forever()
