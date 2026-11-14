#!/usr/bin/env python3
"""Check generated site's local links, anchors and required release content."""
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import urlparse,unquote
ROOT=Path(__file__).resolve().parents[1]/'site'
class Page(HTMLParser):
 def __init__(self):super().__init__();self.links=[];self.ids=set()
 def handle_starttag(self,tag,attrs):
  a=dict(attrs)
  if 'id' in a:
   assert a['id'] not in self.ids,'Duplicate HTML id: '+a['id']
   self.ids.add(a['id'])
  for key in ('href','src'):
   if key in a:self.links.append(a[key])
pages={}
for p in ROOT.rglob('*.html'):
 page=Page();page.feed(p.read_text());pages[p.resolve()]=page
for p,page in pages.items():
 for link in page.links:
  url=urlparse(link)
  if url.scheme or url.netloc:continue
  target=(p.parent/unquote(url.path)).resolve() if url.path else p
  assert target.is_relative_to(ROOT.resolve()),f'Link escapes site: {p}: {link}'
  assert target.exists(),f'Missing link: {p}: {link}'
  if url.fragment and target in pages:assert unquote(url.fragment) in pages[target].ids,f'Missing anchor: {p}: {link}'
text=(ROOT/'index.html').read_text()
for required in ['Power Pudu','power-pudu/egg-of-time','RELEASE CANDIDATE','sustained','Nintendo DS','SHA256SUMS','docs/CREDITS.html']:
 assert required.lower() in text.lower(),'Missing required content: '+required
print(f'PASS: {len(pages)} HTML pages, local links, anchors and release attribution')
