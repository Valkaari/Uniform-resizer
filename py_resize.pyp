import c4d
import os


#v 1.0 
#v 1.01 ajout de l'undo.


from c4d import plugins, utils, bitmaps, gui, documents
from c4d.utils import CompareFloatTolerant
# be sure to use a unique ID obtained from www.plugincafe.com
PLUGIN_ID = 1028518
dlg_pyresize		  		 =   10001
DIALOG_TITLE                 =   10002
ID_VSIZEX                    =   10003
ID_VSIZEY                    =   10004
ID_VSIZEZ                    =   10005
SIZEGROUP                    =   10006
ID_HELP                      =   10007



class ResizeObjectDialog(gui.GeDialog):
    actualSize = c4d.Vector(0.0)


    def FieldActive(self,activate):
        self.Enable(ID_VSIZEX, activate)
        self.Enable(ID_VSIZEY, activate)
        self.Enable(ID_VSIZEZ, activate)

    
    def UpdateSize(self):
        self.doc = c4d.documents.GetActiveDocument()
        if not self.doc: return False
        op = self.doc.GetActiveObject()
        if not op:
            self.FieldActive(False)
            return False
            
        if not (op.CheckType(c4d.Opolygon)) and not (op.CheckType(c4d.Ospline)):
            #op is not a polygon object, desable field
            self.FieldActive(False)
            self.SetReal(ID_VSIZEX, 0.0)
            self.SetReal(ID_VSIZEY, 0.0)
            self.SetReal(ID_VSIZEZ, 0.0)
            self.actualSize = c4d.Vector(0.0)

        else:
            #op is a polygon object, activate field
            self.FieldActive(True)
            opsize = op.GetRad()*2
            opscale  = op.GetAbsScale()
            newsize = c4d.Vector(0.0)
            newsize.x = opsize.x * opscale.x
            newsize.y = opsize.y * opscale.y
            newsize.z = opsize.z * opscale.z

            self.SetReal(ID_VSIZEX, newsize.x)
            self.SetReal(ID_VSIZEY, newsize.y)
            self.SetReal(ID_VSIZEZ, newsize.z)
            self.actualSize = newsize

           
        
    def ChangeSize(self):

        # check if selected object si a polygon object
        self.doc = c4d.documents.GetActiveDocument()
        op = self.doc.GetActiveObject()
        if not op: return False
        if not (op.CheckType(c4d.Opolygon)) and not (op.CheckType(c4d.Ospline)): return False

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
        

        # what dimension have change
        if not CompareFloatTolerant(ratio.x, 1.0):
            ratio = c4d.Vector(ratio.x, ratio.x, ratio.x)
        elif not CompareFloatTolerant(ratio.y, 1.0):
            ratio = c4d.Vector(ratio.y, ratio.y, ratio.y)
        elif not CompareFloatTolerant(ratio.z, 1.0):
            ratio = c4d.Vector(ratio.z, ratio.z, ratio.z)

            
      
        #scale

        
        points = op.GetAllPoints()
        scaledPoints = [ratio.x * point for point in points]

        #set undo 
        self.doc.StartUndo()
        self.doc.AddUndo(c4d.UNDOTYPE_CHANGE, op)
        #update points
        op.SetAllPoints(scaledPoints)
            
        self.doc.EndUndo()
        points = []
        scaledPoints = []

        if op.CheckType(c4d.Ospline):
            for i in xrange(op.GetTangentCount()):
                tans = op.GetTangent(i)
                op.SetTangent(i,tans['vl']* ratio.x, tans['vr']*ratio.x)

        #update cinema4D
        op.Message(c4d.MSG_UPDATE)
        c4d.EventAdd()  


        return True


    def CreateLayout(self):
        # set the title and field of UI
        self.SetTitle(c4d.plugins.GeLoadString(DIALOG_TITLE))
        self.GroupBegin(id = SIZEGROUP, flags = c4d.BFH_SCALEFIT, cols=3,initw = 300)
        self.AddEditNumber(id=ID_VSIZEX, flags =c4d.BFH_SCALEFIT,initw=80)
        self.AddEditNumber(id=ID_VSIZEY, flags =c4d.BFH_SCALEFIT,initw=80)
        self.AddEditNumber(id=ID_VSIZEZ, flags =c4d.BFH_SCALEFIT,initw=80)
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
