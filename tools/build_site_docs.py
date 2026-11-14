#!/usr/bin/env python3
"""Render project documentation for GitHub Pages, preserving source links."""
from pathlib import Path
import html,re,shutil
import markdown
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'site/docs';OUT.mkdir(parents=True,exist_ok=True)
SOURCES=list((ROOT/'docs').glob('*.md'))+[ROOT/'LICENSING.md',ROOT/'upstream/EGG-OF-TIME-LICENSE.md']
for source in SOURCES:
 text=source.read_text()
 title=next((line.lstrip('# ').strip() for line in text.splitlines() if line.startswith('# ')),source.stem)
 def link(match):
  label,target=match.groups()
  if target.startswith(('http://','https://','#','mailto:')):return match.group(0)
  path,sep,fragment=target.partition('#')
  resolved=(source.parent/path).resolve()
  if resolved in SOURCES:target=resolved.stem+'.html'+(sep+fragment if sep else '')
  elif resolved.is_relative_to(ROOT):target='https://github.com/savageforengland-hue/chrono-trigger-PS5/blob/main/'+resolved.relative_to(ROOT).as_posix()+(sep+fragment if sep else '')
  return '['+label+']('+target+')'
 text=re.sub(r'\[([^\]]+)\]\(([^)]+)\)',link,text)
 body=markdown.markdown(text,extensions=['tables','fenced_code','toc'])
 page='<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>'+html.escape(title)+' — CTPS5</title><link rel="icon" href="../assets/icon.png"><link rel="stylesheet" href="../assets/style.css"><link rel="stylesheet" href="../assets/docs.css"></head><body><header class="header"><a class="wordmark" href="../index.html">◷ CTPS5</a><a class="repo-link" href="https://github.com/savageforengland-hue/chrono-trigger-PS5">GitHub →</a></header><main class="doc"><nav class="doc-nav" aria-label="Documentation"><a href="../index.html">← Project home</a><a href="BUILDING.html">Build</a><a href="INSTALLING.html">Install</a><a href="CONTROLS.html">Controls</a><a href="CREDITS.html">Credits</a></nav><article>'+body+'</article><p class="doc-source"><a href="https://github.com/savageforengland-hue/chrono-trigger-PS5/blob/main/'+source.relative_to(ROOT).as_posix()+'">View document source →</a></p></main></body></html>'
 (OUT/(source.stem+'.html')).write_text(page)
for report in (ROOT/'docs').glob('RC*-BUILD-REPORT.json'):
 shutil.copyfile(report,OUT/report.name)
print(f'Rendered {len(SOURCES)} documentation pages')
