from PIL import Image,ImageDraw
from pathlib import Path
import json
root=Path(__file__).resolve().parent.parent;outdir=root/'HP1-HD-Refresh-v0.2';m=json.loads((root/'refresh-work/manifest.json').read_text());rows=m['textures'];cats=sorted({t['category'] for t in rows})
sheet=Image.new('RGB',(900,len(cats)*290+40),(22,24,31));d=ImageDraw.Draw(sheet);d.text((15,10),'Recovered v0.1 HD backup',fill='white');d.text((465,10),'Neural refresh v0.2',fill='white')
for i,cat in enumerate(cats):
 ts=[t for t in rows if t['category']==cat];preferred=['skharrytex0','skrontex0','stone','grass','book','fire']
 t=next((t for term in preferred for t in ts if term in Path(t['path']).name.lower()),ts[0]);y=40+i*290;d.text((15,y),cat+' / '+Path(t['path']).name,fill='white')
 for x,src in [(15,root/'texture-base'/t['path']),(465,root/'refresh-work'/cat/t['path'])]:
  im=Image.open(src).convert('RGBA');im.thumbnail((420,250));bg=Image.new('RGBA',im.size,(55,55,55,255));bg.alpha_composite(im);sheet.paste(bg.convert('RGB'),(x,y+25))
sheet.save(outdir/'HP1-HD-Refresh-Comparison.jpg',quality=94)
