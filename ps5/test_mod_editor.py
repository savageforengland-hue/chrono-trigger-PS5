#!/usr/bin/env python3
"""Exercise the standalone editor with Playwright and inspect generated ZIPs."""
import asyncio,json,zipfile,tempfile
from pathlib import Path
from PIL import Image
from playwright.async_api import async_playwright
ROOT=Path(__file__).resolve().parents[1]
async def main():
 with tempfile.TemporaryDirectory() as tmp:
  d=Path(tmp);png=d/'face.png';Image.new('RGBA',(27,59),(123,44,67,255)).save(png)
  async with async_playwright() as p:
   browser=await p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
   page=await browser.new_page(accept_downloads=True);errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
   await page.set_content((ROOT/'tools/mod-editor/index.html').read_text())
   await page.locator('#png').set_input_files(str(png));await page.wait_for_function("document.querySelector('#assets').textContent.includes('portraits/crono.png')")
   await page.locator('#id').fill('test-theme');await page.locator('#name').fill('Test theme');await page.locator('#name').dispatch_event('input')
   async with page.expect_download() as wait:await page.locator('#themeZip').click()
   file=await wait.value;out=d/'theme.zip';await file.save_as(out)
   with zipfile.ZipFile(out) as z:
    assert z.testzip() is None;prefix='eggoftime/mods/battle-hud/themes/test-theme/'
    assert sorted(z.namelist())==[prefix+'portraits/crono.png',prefix+'theme.json'];assert z.read(prefix+'portraits/crono.png')==png.read_bytes();assert json.loads(z.read(prefix+'theme.json'))['name']=='Test theme'
   folder=d/'edited';dest=folder/'frames/Chara/Chara_0000/000.png';dest.parent.mkdir(parents=True);dest.write_bytes(png.read_bytes())
   await page.locator('#graphics').set_input_files(str(folder));await page.wait_for_function("document.querySelector('#graphicsList').textContent.includes('frames/Chara/Chara_0000/000.png')")
   async with page.expect_download() as wait:await page.locator('#graphicsZip').click()
   file=await wait.value;out=d/'graphics.zip';await file.save_as(out)
   with zipfile.ZipFile(out) as z:
    assert z.testzip() is None;assert z.read('eggoftime/mods/graphics/frames/Chara/Chara_0000/000.png')==png.read_bytes();assert json.loads(z.read('eggoftime/mods/graphics/manifest.generated.json'))['files']==['frames/Chara/Chara_0000/000.png']
   await page.locator('summary').click();await page.locator('#json').fill('{bad json');await page.locator('#apply').click();assert 'JSON' in await page.locator('#status').inner_text() or 'property' in await page.locator('#status').inner_text()
   assert not errors,errors
   await browser.close()
 print('PASS: offline browser PNG selection, generated theme JSON/ZIP and graphics folder ZIP; paths/CRC/PNG bytes verified, malformed JSON reported')
asyncio.run(main())
