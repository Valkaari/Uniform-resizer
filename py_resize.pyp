import c4d
import os


#v 1.0 
#v 1.01 ajout de l'undo.


from c4d import plugins, utils, bitmaps, gui, documents
from c4d.utils import CompareFloatTolerant
# be sure to use a unique ID obtained from www.plugincafe.com
PLUGIN_ID = 1028518
XGROUP                      =   1000
YGROUP                      =   1001
ZGROUP                      =   1002
XSTR                        =   1003
YSTR                        =   1004
ZSTR                        =   1005


dlg_pyresize		  		=   10001
DIALOG_TITLE                =   10002
ID_VSIZEX                   =   10003
ID_VSIZEY                   =   10004
ID_VSIZEZ                   =   10005
SIZEGROUP                   =   10006
ID_HELP                     =   10007



class ResizeObjectDialog(gui.GeDialog):
    actualSize = c4d.Vector(0.0)
    multiObj = False

    def _FieldActive(self,activate):
        self.Enable(ID_VSIZEX, activate)
        self.Enable(ID_VSIZEY, activate)
        self.Enable(ID_VSIZEZ, activate)

    def _CheckObjType(self, objList):
        if not objList:
            return False
        if isinstance(objList, list):
            for obj in objList:
                if not (obj.CheckType(c4d.Opolygon)) and not (obj.CheckType(c4d.Ospline)):
                    return False
        else:
            if not (objList.CheckType(c4d.Opolygon)) and not (objList.CheckType(c4d.Ospline)): 
                return False

        return True

    def _GetObjSize(self, op):
        opsize = op.GetRad()*2
        opscale  = op.GetAbsScale()
        objSize = c4d.Vector(0.0)
        objSize.x = opsize.x * opscale.x
        objSize.y = opsize.y * opscale.y
        objSize.z = opsize.z * opscale.z
        return objSize

    def _SetUI(self, value):
        if isinstance(value, c4d.Vector):
            self.SetReal(ID_VSIZEX, value.x)
            self.SetReal(ID_VSIZEY, value.y)
            self.SetReal(ID_VSIZEZ, value.z)
        else:
            self.SetReal(ID_VSIZEX, value)
            self.SetReal(ID_VSIZEY, value)
            self.SetReal(ID_VSIZEZ, value)
    def UpdateSize(self):
        self.multiObj = False
        self.doc = c4d.documents.GetActiveDocument()
        if not self.doc: return False
        objs = self.doc.GetActiveObjects(c4d.GETACTIVEOBJECTFLAGS_0)
        if len(objs)== 0: 
            self._FieldActive(False)
            return False
        if len(objs) ==1:    
            op = objs[0]
            if not self._CheckObjType(objs):
                #op is not a polygon object, disable field
                self._FieldActive(False)
                self._SetUI(0.0)
                self.actualSize = c4d.Vector(0.0)

            else:
                #op is a polygon object, activate field
                self._FieldActive(True)
                newsize = self._GetObjSize(op)
                self._SetUI(newsize)
                self.actualSize = newsize
        else:
            if not self._CheckObjType(objs):
                #op is not a polygon object, desable field
                self._FieldActive(False)
                self._SetUI(0.0)
                self.actualSize = c4d.Vector(0.0)                
                return False
            self.actualSize = c4d.Vector(0.0)
            self._SetUI(1.0)
            self.multiObj = True
            self._FieldActive(True)
    def _ScaleObject(self,op, ratio):
        points = op.GetAllPoints()
        scaledPoints = [ratio * point for point in points]
        self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, op)
        #update points
        op.SetAllPoints(scaledPoints)
            
        if op.CheckType(c4d.Ospline):
            for i in xrange(op.GetTangentCount()):
                tans = op.GetTangent(i)
                op.SetTangent(i,tans['vl']* ratio, tans['vr']*ratio)
        #update cinema4D
        op.Message(c4d.MSG_UPDATE)

        
    def ChangeSize(self):

        # check if selected object si a polygon object
        self.doc = c4d.documents.GetActiveDocument()
        objList = self.doc.GetActiveObjects(c4d.GETACTIVEOBJECTFLAGS_0)
        if len(objList) == 0: return False
        if not self._CheckObjType(objList):
            return False


        if self.multiObj:
            #for each object calculate the ratio.
            ratioUI = c4d.Vector(1)
            ratioUI.x = self.GetReal(ID_VSIZEX)
            ratioUI.y = self.GetReal(ID_VSIZEY)
            ratioUI.z = self.GetReal(ID_VSIZEZ)
            self.doc.StartUndo()
            for obj in objList:
                ratio = 1
                size = self._GetObjSize(obj)
                if not CompareFloatTolerant(ratioUI.x, 1.0):
                    ratio = ratioUI.x /size.x 
                    self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, obj)
                    self._ScaleObject(obj,ratio)    
                elif not CompareFloatTolerant(ratioUI.y, 1.0):
                    ratio = ratioUI.y / size.y 
                    self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, obj)
                    self._ScaleObject(obj,ratio)    
                elif not CompareFloatTolerant(ratioUI.z, 1.0):
                    ratio = ratioUI.z / size.z 
                    self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, obj)
                    self._ScaleObject(obj,ratio)    
            self.doc.EndUndo()

        else:

            #calculer le ration pour toutes les dimensions éviter les divisions par 0 pour les splines ou objet plat.
            ratio = c4d.Vector(1.0,1.0,1.0)
            if CompareFloatTolerant(self.actualSize.x,0.0):
                ratio.x = 1.0
            else:
                ratio.x = self.GetReal(ID_VSIZEX) / self.actualSize.x

            if CompareFloatTolerant(self.actualSize.y,0.0):
                ratio.y = 1.0
            else:
                ratio.y = self.GetReal(ID_VSIZEY) / self.actualSize.y

            if CompareFloatTolerant(self.actualSize.z,0.0):
                ratio.z = 1.0
            else:
                ratio.z = self.GetReal(ID_VSIZEZ) / self.actualSize.z
            

            # repérer le changement de dimension dans les champs de l'interface.
            if not CompareFloatTolerant(ratio.x, 1.0):
                ratio = c4d.Vector(ratio.x, ratio.x, ratio.x)
            elif not CompareFloatTolerant(ratio.y, 1.0):
                ratio = c4d.Vector(ratio.y, ratio.y, ratio.y)
            elif not CompareFloatTolerant(ratio.z, 1.0):
                ratio = c4d.Vector(ratio.z, ratio.z, ratio.z)
            
            
            self.doc.StartUndo()
            self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, objList[0])
            self._ScaleObject(objList[0],ratio.x)    

            self.doc.EndUndo()

        
        c4d.EventAdd()  


        return True


    def CreateLayout(self):
        # set the title and field of UI
        self.SetTitle(c4d.plugins.GeLoadString(DIALOG_TITLE))
        self.GroupBegin(id = SIZEGROUP, flags = c4d.BFH_SCALEFIT, cols=3,initw = 300)

        self.GroupBegin(id= XGROUP, flags = c4d.BFH_SCALEFIT, cols = 2)
        self.AddStaticText(id= XSTR, flags = c4d.BFH_FIT, initw = 10, name = "X")
        self.AddEditNumber(id=ID_VSIZEX, flags =c4d.BFH_SCALEFIT,initw=80)
        self.GroupEnd()

        self.GroupBegin(id= YGROUP, flags = c4d.BFH_SCALEFIT, cols = 2)
        self.AddStaticText(id= YSTR, flags = c4d.BFH_FIT, initw = 10, name = "Y")
        self.AddEditNumber(id=ID_VSIZEY, flags =c4d.BFH_SCALEFIT,initw=80)
        self.GroupEnd()

        self.GroupBegin(id= ZGROUP, flags = c4d.BFH_SCALEFIT, cols = 2)
        self.AddStaticText(id= ZSTR, flags = c4d.BFH_FIT, initw = 10, name = "Z")
        self.AddEditNumber(id=ID_VSIZEZ, flags =c4d.BFH_SCALEFIT,initw=80)
        self.GroupEnd()        
        

        self.GroupEnd()
        return True
    
           
    def CoreMessage(self, id, msg):
        if id ==c4d.EVMSG_CHANGE:
            #something have change in cinema4D, maybe the object selected isn't the same.

            self.UpdateSize()
        
        return True
    
    def InitValues(self):
        self.UpdateSize()
        return True

    def Command(self, id, msg):
        if id == ID_VSIZEX or id == ID_VSIZEY or id == ID_VSIZEZ:
            self.ChangeSize()
        return True

    
    

class ResizeObject (plugins.CommandData):
    dialog = None;
    
    def Execute (self, doc):
        if self.dialog is None:
            self.dialog = ResizeObjectDialog()
        return self.dialog.Open(dlgtype = c4d.DLG_TYPE_ASYNC, pluginid = PLUGIN_ID, defaultw = 300, defaulth = 32)
    

    
    
    def RestoreLayout(self, sec_ref):
        if self.dialog is None:
            self.dialog = ResizeObjectDialog()
        return self.dialog.Restore(pluginid = PLUGIN_ID, secret = sec_ref)







if __name__ == "__main__":
    # load icon.tif from res into bmp
    bmp = bitmaps.BaseBitmap()
    dir, file = os.path.split(__file__)
    fn = os.path.join(dir, "res","icons", "pyresize.tif")
    bmp.InitWith(fn)
    # register the plugin
    plugins.RegisterCommandPlugin(id=PLUGIN_ID, 
                                  str="Py-resize",
                                  info=0,
                                  help=c4d.plugins.GeLoadString(ID_HELP), 
                                  dat=ResizeObject(),
                                  icon=bmp)
