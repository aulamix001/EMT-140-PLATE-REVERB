#!/usr/bin/env python3
"""Deterministic original sound-design bank. No proprietary preset extraction."""
from pathlib import Path
import csv,json,xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1]
IDS=['plate_select','reverb_time_a','reverb_time_b','reverb_time_c','input_filter_model','input_filter_emt_db','input_filter_martech_hz','width','balance','eq_enable','eq_low_freq','eq_low_gain','eq_high_freq','eq_high_gain','mod_rate','mod_depth','predelay','mix','wet_solo','power']
DEFAULT=[0,2,2,2,0,1,2,100,0,1,120,0,8000,0,0,0,50,15,1,1]
bank=[]
def add(name,category,plate,rt,model,fil,width,lf,lg,hf,hg,rate,depth,pre,mix,solo,description):
 v=DEFAULT.copy();v[0]=plate;v[plate+1]=rt;v[4]=model;v[6 if model==1 else 5]=fil
 for i,x in zip([7,10,11,12,13,14,15,16,17,18],[width,lf,lg,hf,hg,rate,depth,pre,mix,int(solo)]):v[i]=x
 bank.append(dict(number=len(bank)+1,name=name,category=category,use='AUX' if solo else 'INSERT',description=description,parameters=dict(zip(IDS,v))))
legacy=[
('Vocal Long','Vocais',0,2.8,0,1,100,120,-1.5,8000,1.5,0,0,20,100,1),('Vocal Tight','Vocais',0,1.6,0,0,100,120,0,10000,2,0,0,10,100,1),('Vocal Legacy','Vocais',0,2,0,1,90,120,0,8000,0,0,0,0,100,1),
('Drum Room','Bateria',1,.9,0,1,80,150,-3,8000,0,0,0,0,100,1),('Percussion Short','Bateria',1,.6,0,2,60,120,0,8000,0,0,0,0,100,1),('Strings Bright','Cordas',2,2.4,1,2,100,120,0,8000,2,0,0,25,100,1),('Dark Jazz','Cordas',2,1.8,1,0,100,200,2,4000,-2,0,0,15,100,1),
('Ambient Wide','Atmosferas',0,4.5,0,0,100,120,0,8000,0,.2,6,40,100,1),('Cinematic 5s','Cinema',1,5,1,1,100,120,0,8000,0,0,0,80,100,1),('Modulated Chorus','Atmosferas',2,3,1,3,100,120,0,8000,0,.8,10,10,100,1),('Insert Mono','Mix',1,1.2,0,1,0,120,0,8000,0,0,0,0,25,0),('Mastering Glue','Mix',2,2.2,1,1,100,100,-.5,12000,.5,0,0,30,8,0)]
for x in legacy:add(*x,'Preset de referencia da especificacao fornecida; valores originais preservados.')
# name, plate, decay seconds, predelay ms, width %, tonal recipe, usage mix (100=AUX)
groups={
'Vocais':[
('Lead Intimate',0,1.1,18,85,'warm',12),('Lead Pop Air',2,1.9,38,100,'air',100),('Ballad Velvet',0,3.2,65,100,'warm',100),('Rap Short Plate',1,.7,35,75,'clean',10),('Soul Golden',0,2.3,28,95,'warm',100),('Jazz Smoky',0,1.7,22,80,'dark',100),('Backing Silk',2,2.6,12,100,'air',100),('Choir Cathedral Plate',0,4.2,55,100,'smooth',100),('Spoken Close',1,.5,8,45,'clean',7),('Sibilance Tamer',0,1.5,42,90,'dark',100),('Vocal Dream',2,3.8,70,100,'mod',100),('Vintage Slap Bloom',1,1.3,115,85,'warm',14)],
'Bateria':[
('Snare Chrome',1,1.2,12,90,'bright',100),('Snare Eighties',2,2.5,55,100,'air',100),('Snare Tight Dark',1,.5,0,60,'dark',100),('Rimshot Space',1,.8,8,75,'clean',100),('Toms Thunder',0,1.8,14,95,'warm',100),('Brushes Velvet',0,1.2,5,75,'smooth',100),('Kick Upper Plate',1,.6,2,40,'thin',100),('Clap Spread',2,1.1,18,100,'bright',100),('Shaker Dust',1,.7,0,80,'dark',100),('Congas Copper',0,1.4,9,85,'warm',100),('Drum Parallel',1,.9,6,70,'clean',9),('Cymbal Mist',2,2.1,25,100,'smooth',100)],
'Guitarras':[
('Acoustic Wood',0,1.4,18,80,'warm',12),('Acoustic Sparkle',2,1.9,25,100,'air',100),('Nylon Intimate',0,1.2,15,70,'smooth',10),('Electric Surf Plate',2,2.7,6,100,'bright',22),('Blues Smoky',0,1.8,32,85,'dark',14),('Clean Arpeggio',2,2.2,42,100,'clean',100),('Lead Sustain',0,3.1,48,95,'smooth',100),('Heavy Guitar Depth',1,.8,28,60,'thin',6),('Slide Moonlight',0,3.4,60,100,'mod',100),('Funk Springy Plate',1,.9,10,75,'bright',11)],
'Teclas':[
('Piano Felt',0,1.8,25,85,'warm',12),('Piano Concert Plate',2,3.2,40,100,'clean',100),('Rhodes Honey',0,2.4,22,95,'warm',16),('Wurli Amber',1,1.3,15,80,'dark',12),('Organ Chapel',0,3.6,35,100,'smooth',100),('Clav Snap',1,.6,5,65,'thin',8),('Synth Pluck Halo',2,2.1,30,100,'bright',100),('Analog Pad Drift',0,4.6,70,100,'mod',100),('FM Bell Shimmer',2,3.7,48,100,'air',100),('Celesta Stardust',2,2.8,22,100,'smooth',100)],
'Cordas':[
('Violin Solo Gold',0,2.3,28,85,'warm',100),('Cello Chamber',0,1.9,24,75,'smooth',100),('String Ensemble Silk',2,3.1,35,100,'air',100),('Pizzicato Pearl',1,.9,12,80,'bright',100),('Double Bass Shadow',1,1.1,18,55,'thin',8),('Harp Silver',2,3.5,45,100,'clean',100),('Quartet Vintage',0,2.1,20,90,'warm',100),('Orchestral Swell',0,4.1,60,100,'smooth',100)],
'Sopros':[
('Trumpet Gold',0,1.6,28,80,'warm',100),('Muted Trumpet Noir',0,2.2,42,85,'dark',100),('Tenor Sax Smoke',0,2,32,85,'smooth',100),('Alto Sax Shine',2,1.7,25,90,'air',100),('Trombone Body',1,1.4,22,75,'thin',100),('Brass Section Wide',2,2.5,38,100,'bright',100),('Flute Breeze',2,2.8,35,100,'smooth',100),('Clarinet Wood',0,1.8,24,80,'warm',13)],
'Atmosferas':[
('Infinite Steel',0,5.5,110,100,'mod',100),('Frozen Amber',0,5.2,85,100,'dark',100),('Silver Cloud',2,4.8,95,100,'air',100),('Slow Aurora',2,5,150,100,'mod',100),('Dream Chamber',0,3.9,65,100,'smooth',100),('Metallic Stars',2,4.3,45,100,'bright',100),('Narrow Memory',0,3.3,90,30,'dark',100),('Mono Ghost',1,2.9,125,0,'warm',100),('Soft Horizon',0,4.7,75,95,'smooth',100),('Analog Nebula',2,5.3,180,100,'mod',100)],
'Cinema':[
('Trailer Steel Tail',1,5.4,120,100,'bright',100),('Noir Corridor',0,3.6,75,65,'dark',100),('SciFi Reactor',2,4.9,135,100,'mod',100),('Distant Footsteps',1,1.7,85,55,'thin',100),('Dialogue Depth',1,.8,20,50,'clean',6),('Cinematic Drum Bloom',0,3.2,55,100,'warm',100),('Dream Transition',2,4.4,160,100,'smooth',100),('Vintage Radio Space',1,1.3,15,35,'dark',18),('Impact Afterglow',0,5.1,200,100,'thin',100),('Final Frame',2,5.5,250,100,'air',100)],
'Mix':[
('Mix Bus Subtle',2,.8,18,90,'clean',4),('Drum Bus Glue',1,.6,8,70,'warm',5),('Mono Compatibility',0,1.4,20,0,'smooth',10),('Parallel Brightness',2,1.2,12,100,'air',100),('Vintage Cohesion',0,1.6,25,85,'warm',7),('Master Dark Space',0,1.1,30,90,'dark',3),('Wide Instrument Glue',2,1.8,35,100,'clean',8),('Aux Neutral Plate',0,2.2,25,100,'clean',100)]}
# Tonal recipes attenuate low-end on wet return, with no constant hiss or saturation control.
tones={'warm':(0,1,140,-1.5,6000,-1,0,0),'air':(1,2,160,-2,10000,2,0,0),'clean':(0,1,120,-1,8000,0,0,0),'dark':(1,1,180,-1,4000,-4,0,0),'smooth':(0,1,150,-1.5,7000,-1.5,0,0),'mod':(1,2,180,-2,8000,.5,.18,4),'bright':(1,3,200,-2.5,8000,2.5,0,0),'thin':(1,5,250,-4,6000,-1,0,0)}
for category,rows in groups.items():
 for name,plate,rt,pre,width,tone,mix in rows:
  model,fil,lf,lg,hf,hg,rate,depth=tones[tone]
  add(name,category,plate,rt,model,fil,width,lf,lg,hf,hg,rate,depth,pre,mix,mix==100,f'{category}; carater {tone}; RT {rt:g} s; predelay {pre:g} ms; '+('retorno auxiliar 100% wet.' if mix==100 else f'insert com {mix}% wet.'))
assert len(bank)==100,len(bank)
assert len({tuple(x['parameters'].values()) for x in bank})==100
out=ROOT/'Presets/PLATE140-100';out.mkdir(parents=True,exist_ok=True)
for p in bank:
 root=ET.Element('PLATE140',version='1',preset_name=p['name'],preset_category=p['category'],preset_number=str(p['number']))
 for id,value in p['parameters'].items():ET.SubElement(root,'PARAM',id=id,value=f'{value:g}')
 ET.indent(root);ET.ElementTree(root).write(out/f"{p['number']:03d}_{p['name'].replace(' ','_')}.xml",encoding='utf-8',xml_declaration=True)
(out/'Bank.json').write_text(json.dumps(dict(format='PLATE140-BANK',version=1,presets=bank),indent=2)+'\n')
with (out/'Mapeamento.csv').open('w',newline='',encoding='utf-8-sig') as f:
 w=csv.writer(f,delimiter=';',lineterminator='\n');w.writerow(['Numero','Nome','Categoria','Uso',*IDS]);
 for p in bank:w.writerow([p['number'],p['name'],p['category'],p['use'],*p['parameters'].values()])
h=['#pragma once','#include <array>','namespace platebank {','inline constexpr const char* ids[] = {'+','.join(json.dumps(x) for x in IDS)+'};','struct Preset {const char* name; const char* category; std::array<float,20> values;};','inline constexpr Preset presets[] = {']
for p in bank:h.append('{'+json.dumps(p['name'])+','+json.dumps(p['category'])+',{{'+','.join(f'{float(x):.6f}f' for x in p['parameters'].values())+'}}},')
h.extend(['};','inline constexpr int count=sizeof(presets)/sizeof(presets[0]);','static_assert(count==100);','}'])
(ROOT/'Source/Presets/FactoryPresets.h').write_text('\n'.join(h)+'\n')
print('Generated 100 unique presets, 100 XML, bank JSON, full CSV and embedded C++ bank.')
