//
//  obj_resize.hpp
//  objectresize
//
//  Created by Manuel MAGALHAES on 22/08/2017.
//  Copyright © 2017 MAXON Computer GmbH. All rights reserved.
//

#ifndef obj_resize_hpp
#define obj_resize_hpp

#include <stdio.h>


class ObjectResizeDialog: public GeDialog
{
    INSTANCEOF(ObjectResizeDialog, GeDialog);
    
    
    
public:
    virtual Bool                    InitValues(void);
    virtual Bool                    CreateLayout();
    virtual Bool                    CoreMessage (Int32 id, const BaseContainer &msg);
    virtual Bool                    Command(Int32 id,const BaseContainer & msg );

private :
    
//    Vector  actualSize_; ///< actual size of the object 0 if error 1 if multiple object selected.
    
    
    
    Float GetRatio(Vector actualSize );
    
    //----------------------------------------------------------------------------------------
    ///update the UI with the size of object or disable fields
    ///@return true if success
    //----------------------------------------------------------------------------------------
    Bool UpdateUI_(void);
    //----------------------------------------------------------------------------------------
    ///set the fields value
    ///@param[in] sizeX the size in the X direction
    ///@param[in] sizeY the size in the Y direction
    ///@param[in] sizeZ the size in the Z direction
    ///@return true if ok
    //----------------------------------------------------------------------------------------
    Bool SetUIValue_ (Float sizeX, Float sizeY, Float sizeZ, Bool tristate = false);
    
    
    //----------------------------------------------------------------------------------------
    ///activate or desactivate the ui
    ///@param[in] status true if the field must be activated of false if not
    //----------------------------------------------------------------------------------------
    void ActivateField_(Bool status);
    
    //----------------------------------------------------------------------------------------
    ///check if the object type in the array are opolygon or opsline
    ///@param[in] objList the list of objects to check
    ///@return false if something is not sure
    //----------------------------------------------------------------------------------------
    Bool CheckObjectType_ (AtomArray *objList);
    
    //----------------------------------------------------------------------------------------
    ///Get the size of the object
    ///@param[in] op the object to get the size from
    ///@return the size in a vector form
    //----------------------------------------------------------------------------------------
    Vector GetObjectSize_(BaseObject* op);
    
    //----------------------------------------------------------------------------------------
    ///scale the object with the ratio
    ///@param[in] op the object to resize
    ///@param[in] ratio the ratio
    ///@return true if success
    //----------------------------------------------------------------------------------------
    Bool ScaleObject_(BaseObject* op,Float &ratio);
    
    //----------------------------------------------------------------------------------------
    ///Scale one or several objects.
    ///@return true objects have been resized
    //----------------------------------------------------------------------------------------
    Bool ModifyScaleObject_(void);
    
    
    //----------------------------------------------------------------------------------------
    ///Get the bounding box of the selected elements
    ///@param[in] op the object the selection is comming from.
    ///@param[in] mode the kind of selection to check.
    ///@return Vector the size.
    //----------------------------------------------------------------------------------------
    Vector GetSelectionSize_(C4DAtom* op, Int32 mode);
    
    
    
    //----------------------------------------------------------------------------------------
    ///Scale the selection
    ///@return true if success
    //----------------------------------------------------------------------------------------
    
    Bool ModifyScaleSelection_(void);
    
    
    //----------------------------------------------------------------------------------------
    ///main fonction that the document's mode and call the according function.
    ///@return true or false.
    //----------------------------------------------------------------------------------------
    Bool Modification_(void);
    
};


class ObjectResizeCommand : public CommandData
{
    INSTANCEOF(ObjectResizeCommand, CommandData)
    
    
public:
    
    virtual Bool Execute(BaseDocument* doc);
    virtual Bool RestoreLayout(void* secret);
    
    static ObjectResizeCommand* Alloc() { return NewObjClear(ObjectResizeCommand); }
    
private:
    ObjectResizeDialog dlg_;
    
    
};



#endif /* obj_resize_hpp */
