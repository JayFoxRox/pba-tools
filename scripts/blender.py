# Invoke using "blender --python blender.py -- <path-to-table file>"

import bpy
import os
import json
import sys

argv = sys.argv
argv = argv[argv.index("--") + 1:]

#FIXME: Parse arguments

context = bpy.context

data_path = argv[0]

print("Path: '" + data_path + "'")

#context.space_data.show_backface_culling = True


#create a scene
scene = bpy.data.scenes.new("Import")
camera_data = bpy.data.cameras.new("Camera")

camera = bpy.data.objects.new("Camera", camera_data)
camera.location = (-2.0, 3.0, 3.0)
camera.rotation_euler = (422.0, 0.0, 149)
scene.objects.link(camera)

# do the same for lights etc

scene.update()

# make a new scene with cam and lights linked
scene.camera = camera
context.screen.scene = scene
context.scene.render.engine = 'BLENDER_GAME'

def fileContents(filename):
    with open(filename) as f:
        return f.read()


enums = json.loads(fileContents(os.path.join(data_path, "./enums.json"))) # Load _Enums file [as json]
objs  = json.loads(fileContents(os.path.join(data_path, "./table.json"))) # Load table file [as json]
dil   = json.loads(fileContents(os.path.join(data_path, "./dil.json")))   # Load Placement file [as json]

def resourceIndexFromName(name):
  return enums[name]['index']

def placementFromName(name):
  for placement in dil['objects']:
    a = placement['name']
    b = name
    if a == b:
      return placement
    if a.lower() == b.lower():
      print("Case does not match!")
      return placement
  print("Could not find '" + name + "'")
  return None

def layer(index):
  lm = [False] * 20
  lm[index] = True
  return lm

f = 0.001
def importMesh(resourceName, fileIndex):
  resourceIndex = resourceIndexFromName(resourceName)
  filename = str(resourceIndex) + "-" + str(fileIndex) + ".obj"

  path = os.path.join(data_path, "./converted/" + filename)
  print("Loading " + filename)

  # for collisions [floating vertices are kept!]:
  #bpy.ops.import_scene.obj(filepath=path, split_mode='OFF', use_split_objects=True, axis_forward='Y', axis_up='Z', use_image_search=False, filter_glob="*.obj;*.mtl")
  # for visuals:
  bpy.ops.import_scene.obj(filepath=path, split_mode='ON', use_split_groups=True, axis_forward='Y', axis_up='Z', use_image_search=False, filter_glob="*.obj;*.mtl")

  imported = bpy.context.selected_objects



  print("imported:" + str(imported))
  #bpy.data.objects[]
  for i in imported:
    i.scale *= f

  return imported

def dilPlaceInstance(imported, placement):
  for i in imported:
    p = placement['position']
    xyz = (f * p[0], f * p[1], f * p[2])
    print(str(xyz))
    i.location = xyz
    r = placement['angle']
    i.rotation_mode = 'YXZ';
    i.rotation_euler = (r[0], r[2], r[1])
    s = placement['scale']
    #i.scale = (f * s[0], f * s[1], f * s[2]) # Doesn't seem to be used?!

def dilPlace(imported, obj):

  # Check if this is placed using DIL or directly
  try:
    dilPos = [d['data'] for d in obj if d['type'] == 'DilPos'][0] #FIXME: Turn this into a function!
  except:
    dilPos = None

  if dilPos == None:
    #FIXME: TODO
    print("Want this at standard loc [" + str(obj) + "]")
    placement = [d['data'] for d in obj if d['type'] == 'Pos'][0]
    dilPlaceInstance(imported, placement)
        #"Pos": {
        #    "position": [ -620.434143, 1590.591309, -200.085876 ],
        #    "scale": [ 1.000000, 1.000000, 1.000000 ],
        #    "angle": [ 0.000000, 0.000000, 0.000000 ]
        #},

  else:
    #FIXME: TODO
    print("pos: " + str(dilPos) + " for " + str(obj))
    var = placementFromName(dilPos['variable'])
    if var == None:
      #FIXME: Delete object now?!
      return
    placements = var['placements']
    for placement in placements:
      print("Want this at " + str(placement))
      # FIXME: Place this object at all locations?!
    placement = placements[0]
    dilPlaceInstance(imported, placement)

#print(objs)
  

# Iterate over objects and find PBObject entries
for key in objs:
  obj = objs[key]
  if obj['type'] != "PBObject":
    continue
  obj = obj['data']

  for d in obj:

    if d['type'] == "Object":
      print("Object: " + str(obj))

      obj_object = d['data']
      t = obj_object['type']

      print("Parsing " + key)

      # Mark bad objects

      bad = True
      l = 9

      # Visual only
      if t == "PB_OBJECTTYPE_VISUAL": l = 0

      # Collision and colliding trigger
      if t == "PB_OBJECTTYPE_COLLISION": l = 0
      if t == "PB_OBJECTTYPE_TRAP": l = 0

      # Playfield
      if t == "PB_OBJECTTYPE_FLOOR": l = 1

      # Lamps
      if t == "PB_OBJECTTYPE_LAMPSET": l = 2

      # Unknown
      if t == "PB_OBJECTTYPE_UNIQUE": l = 3

      # Game elements
      if t == "PB_OBJECTTYPE_PLUNGER": l = 4
      if t == "PB_OBJECTTYPE_FLIPPER": l = 4
      if t == "PB_OBJECTTYPE_SLINGSHOT": l = 4
      if t == "PB_OBJECTTYPE_POPBUMPER": l = 4
      if t == "PB_OBJECTTYPE_SPINNER": l = 4
      if t == "PB_OBJECTTYPE_TARGET": l = 4
      if t == "PB_OBJECTTYPE_KICKER": l = 4
      if t == "PB_OBJECTTYPE_STOPPER": l = 4

      if t == "PB_OBJECTTYPE_DIVERTER_ONOFF": l = 4 # FIXME: Does this also have visuals?!

      # Any object on layer >= 5 doesn't have a collision!

      # Game elements (which should be without collision)
      if t == "PB_OBJECTTYPE_GATE": l = 5
      if t == "PB_OBJECTTYPE_GATE_ONEWAY": l = 5
      if t == "PB_OBJECTTYPE_WIRE": l = 5

      # Triggers
      if t == "PB_OBJECTTYPE_MAGNET": l = 6
      if t == "PB_OBJECTTYPE_OPTO": l = 7
      if t == "PB_OBJECTTYPE_BALLDRAIN": l = 8





    #Not rendering: PB_OBJECTTYPE_BALLDRAIN
    #Not rendering: PB_OBJECTTYPE_COLLISION
    #Not rendering: PB_OBJECTTYPE_DIVERTER_ONOFF
    #Not rendering: PB_OBJECTTYPE_FLIPPER
    #Not rendering: PB_OBJECTTYPE_FLOOR
    #Not rendering: PB_OBJECTTYPE_GATE
    #Not rendering: PB_OBJECTTYPE_GATE_ONEWAY
    #Not rendering: PB_OBJECTTYPE_KICKER
    #Not rendering: PB_OBJECTTYPE_LAMPSET
    #Not rendering: PB_OBJECTTYPE_MAGNET
    #Not rendering: PB_OBJECTTYPE_OPTO
    #Not rendering: PB_OBJECTTYPE_PLUNGER
    #Not rendering: PB_OBJECTTYPE_PLUNGER_EXIT
    #Not rendering: PB_OBJECTTYPE_POPBUMPER
    #Not rendering: PB_OBJECTTYPE_SLINGSHOT
    #Not rendering: PB_OBJECTTYPE_SPINNER
    #Not rendering: PB_OBJECTTYPE_STOPPER
    #Not rendering: PB_OBJECTTYPE_TARGET
    #Not rendering: PB_OBJECTTYPE_TRAP
    #Not rendering: PB_OBJECTTYPE_UNIQUE
    #Not rendering: PB_OBJECTTYPE_VISUAL
    #Not rendering: PB_OBJECTTYPE_WIRE


      #if t == "PB_OBJECTTYPE_WIRE": l = 3
      #if t == "PB_OBJECTTYPE_WIRE": l = 3
      #if t == "PB_OBJECTTYPE_WIRE": l = 3
      #if t == "PB_OBJECTTYPE_WIRE": l = 3
      if bad:
        print("Not rendering: " + t)
        #for i in imported:
        #  i.layers = [False] * 20
        #  i.layers[19] = True
        #continue

    if d['type'] == 'Collision':
      collision = d['data']
      ct = collision['type']
      if ct == 'Sphere':
          #FIXME: collision['mode'] should be "Manual"
          x,y,z = collision['position']
          radius = collision['radius']
          #FIXME: Create a new sphere object
          #for i in imported:
          #  i.layers[0] = False
          #  i.layers[1] = True
          #continue
          #imported = 

          bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, size=f * radius, view_align=False, enter_editmode=False, location=(f * x, f * y, f * z))
          bpy.ops.object.shade_smooth()
          imported = bpy.context.selected_objects

          # FIXME: Set physics type to sphere

      elif ct == 'Mesh':
        imported = []
        for mesh in collision['data']:
          imported.extend(importMesh(mesh['resource'], mesh['index']))
          print("Imported append: " + str(imported))
      else:
        print("Damnit! Unsupported collision type: '" + ct + "'")
        imported = None

      if imported:
        print(key + " => " + str(imported))
        dilPlace(imported, obj)
        for i in imported:
          i.layers = layer(l + 10)
          i.hide_render = True
          i.draw_type = 'SOLID'
          if l >= 5:
            i.game.physics_type = 'NO_COLLISION'
            

    if d['type'] == 'Models':
      for model in d['data']:   

        imported = importMesh(model['resource'], model['index'])
        for i in imported:
          i.layers = layer(l)
          i.game.physics_type = 'NO_COLLISION'

        for i in imported:
          for m in i.material_slots:
            print("Slot: " + str(m))
            m.material.game_settings.alpha_blend = 'ALPHA'
            #FIXME: Only do this if the linked texture has alpha.. otherwise this breaks >.<      

        #meshes = [c for c in imported if c.type == 'MESH']
        #for m in meshes:
        #    m.scale *= 0.001

        dilPlace(imported, obj)

        

# Resize the entire scene
# FIXME: Should be done while creating the scene or by setting a different scale of the scene

# FIXME: Set material to alpha

# Only show visuals
scene.layers = [True] * 10 + [False] * 10

print("dir: " + str(scene.game_settings.gravity))
print("used: " + str(scene.use_gravity))
#scene.game_settings.physics_gravity = (0.0, -0.5, -10.0) # Bugs out!

if False:
    cams = [c for c in context.scene.objects if c.type == 'CAMERA']
    for c in cams:
        context.scene.camera = c                                    
        print("Render ", model_path, context.scene.name, c.name)
        context.scene.render.filepath = "somepathmadeupfrommodelname"
        bpy.ops.render.render(write_still=True)
