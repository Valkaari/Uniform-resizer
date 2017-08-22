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
    
    Bool    multiObject_; ///< multi object are selected.
    Vector  actualSize_; ///< actual size of the object 0 if error 1 if multiple object selected.
    
    
    
    //----------------------------------------------------------------------------------------
    ///update the UI with the size of object or disable fields
    ///@return true if success
    //----------------------------------------------------------------------------------------
    Bool UpdateUI_(void);
    
    //----------------------------------------------------------------------------------------
    ///activate or desactivate the ui
    ///@param[in] status true if the field must be activated of false if not
    //----------------------------------------------------------------------------------------
    void ActivateField(Bool status);
    
    //----------------------------------------------------------------------------------------
    ///check if the object type in the array are opolygon or opsline
    ///@param[in] objList the list of objects to check
    ///@return false if something is not sure
    //----------------------------------------------------------------------------------------
    Bool CheckObjectType_ (AtomArray *objList);
    
    
    //----------------------------------------------------------------------------------------
    ///set the fields value
    ///@param[in] sizeX the size in the X direction
    ///@param[in] sizeY the size in the Y direction
    ///@param[in] sizeZ the size in the Z direction
    ///@return true if ok
    //----------------------------------------------------------------------------------------
    Bool SetUIValue_ (Float sizeX, Float sizeY, Float sizeZ);
    
    
    
    Vector GetSelectionSize_(BaseSelect *bs);
    
    
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
    ///called if the user entered a size in the ui.
    ///@return true objects have been resized
    //----------------------------------------------------------------------------------------
    Bool SizeChanged_(void);
    
    
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
