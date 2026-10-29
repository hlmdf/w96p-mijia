"""Fetch the pinned official PlatformIO package, verifying its registry SHA256."""
import concurrent.futures
import hashlib
import json
from pathlib import Path
from platformio.registry.mirror import RegistryFileMirrorIterator
from platformio.http import HTTPClient, HTTPSession

root=Path(__file__).resolve().parents[1]
metadata=root/'framework-package.json'
if not metadata.exists():
    response=HTTPSession().get('https://api.registry.platformio.org/v3/packages/platformio/tool/framework-arduinoespressif32',timeout=30)
    response.raise_for_status();metadata.write_text(response.text)
package=json.loads(metadata.read_text())
version=next(v for v in package['versions'] if v['name']=='3.20017.241212+sha.dcc1105b')
file=version['files'][0]
it=RegistryFileMirrorIterator(file['download_url'])
it._mirror='https://dl.registry.nm1.platformio.org'
it.HTTP_CLIENT_INSTANCES[it._mirror]=HTTPClient([it._mirror])
url, checksum=next(it)
print('Official mirror selected; checking range downloads',flush=True)
size=file['size']
session=HTTPSession()
probe=session.get(url,headers={'Range':'bytes=0-0'},timeout=20)
if probe.status_code!=206:
    raise RuntimeError('Mirror does not support safe range downloading')
probe.close()
folder=root/'.downloads';folder.mkdir(exist_ok=True)
count=8
def fetch(i):
    first=i*size//count;last=(i+1)*size//count-1
    target=folder/f'framework.part{i}'
    if target.exists() and target.stat().st_size==last-first+1:return target
    response=HTTPSession().get(url,headers={'Range':f'bytes={first}-{last}'},stream=True,timeout=(12,45))
    if response.status_code!=206 or response.headers.get('Content-Range')!=f'bytes {first}-{last}/{size}':
        raise RuntimeError('Invalid range response')
    with target.open('wb') as f:
        for b in response.iter_content(131072):f.write(b)
    if target.stat().st_size!=last-first+1:raise RuntimeError('Incomplete chunk')
    print(f'Chunk {i+1}/{count} complete',flush=True)
    return target
with concurrent.futures.ThreadPoolExecutor(max_workers=count) as pool:
    pieces=list(pool.map(fetch,range(count)))
target=folder/'framework.tar.gz';h=hashlib.sha256()
with target.open('wb') as out:
    for p in pieces:
        with p.open('rb') as src:
            while b:=src.read(1048576):out.write(b);h.update(b)
if h.hexdigest()!=checksum or checksum!=file['checksum']['sha256']:
    raise RuntimeError('Checksum mismatch')
print('Framework download verified: SHA256 matches official registry',flush=True)
