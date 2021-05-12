//
//  obj_resize.cpp
//  objectresize
//
//  Created by Manuel MAGALHAES on 22/08/2017.
//  Copyright © 2017 MAXON Computer GmbH. All rights reserved.
//

#include "c4d.h"
#include "c4d_symbols.h"

#include "c4d_snapdata.h"


#include "obj_resize.hpp"

#define VERSION 1.4

#define ID_OBJECT_RESIZE 1028518


Bool ObjectResizeDialog::CheckObjectType_(AtomArray *objList)
{
    if (!objList)
        return false;
    if (objList->GetCount()==0)
        return false;
    
    if (objList->GetCount() == 1  && ToPoly(objList->GetIndex(0))->IsInstanceOf(Onull))
        return true;

    BaseObject* obj = nullptr;
  
    for (Int32 i = 0; i < objList->GetCount(); i++)
    {
        obj = (BaseObject*)objList->GetIndex(i);
        if (!obj->IsInstanceOf(Opolygon) && ((obj->GetInfo() & OBJECT_ISSPLINE) != OBJECT_ISSPLINE))
           return false;
    }
    return true;
}

void ObjectResizeDialog::ActivateField_(Bool status)
{
    
    Enable(ID_VSIZEX, status);
    Enable(ID_VSIZEY, status);
    Enable(ID_VSIZEZ, status);
    Enable(LOCKX, status);
    Enable(LOCKY, status);
    Enable(LOCKZ, status);
    UpdateSizeField_();
}

Bool ObjectResizeDialog::UpdateSizeField_()
{
   
    Bool bLockX;
    Bool bLockY;
    Bool bLockZ;
    GetBool(LOCKX, bLockX);
    GetBool(LOCKY, bLockY);
    GetBool(LOCKZ, bLockZ);
    if (IsEnabled(LOCKX))
        Enable(ID_VSIZEX, bLockX);
    if (IsEnabled(LOCKY))
        Enable(ID_VSIZEY, bLockY);
    if (IsEnabled(LOCKZ))
        Enable(ID_VSIZEZ, bLockZ);
    
    return true;
}

Bool ObjectResizeDialog::GetUVWSelectedSize(C4DAtom *obj , Vector &size, Vector &center)
{
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return false;
    TempUVHandle* handle = GetActiveUVSet(doc, GETACTIVEUVSET_ALL);
    if (handle)
    {
        PolygonObject* op = (PolygonObject*) obj;
        LMinMax bb;
        bb.Init();
        UVWStruct* uvwAddr = handle->GetUVW();
        if (!uvwAddr)
            return false;
        
        if (doc->GetMode()==Muvpoints)
        {
            BaseSelect* bsp = handle->GetUVPointSel();
            for (Int32 i = 0;  i < op->GetPolygonCount() * 4; i++)
                if (bsp->IsSelected(i))
                    bb.AddPoint((Vector)uvwAddr[i/4][i % 4]);
            
            
        }
        else if (doc->GetMode()==Muvpolygons)
        {
            BaseSelect* bs = handle->GetPolySel();
            for (Int32 i = 0 ; i < op->GetPolygonCount();i++)
                if (bs->IsSelected(i))
                {
                    bb.AddPoints(uvwAddr[i].a, uvwAddr[i].b);
                    bb.AddPoints(uvwAddr[i].c, uvwAddr[i].d);
                }
            
        }
        
        FreeActiveUVSet(handle);
        size = bb.GetRad()*2;
        center = bb.GetMp();
        return true;
        
    }
    FreeActiveUVSet(handle);
    return false;
    
}

Bool ObjectResizeDialog::SetUItxt_(Int32 state)
{
    if (state == 0)
    {

		
        if (!SetString(XSTR,maxon::String("X")))
            return false;
        if (!SetString(YSTR, maxon::String("Y")))
            return false;
        if (!SetString(ZSTR, maxon::String("Z")))
            return false;
    }
    
    if (state == 1)
    {
        if (!SetString(XSTR, maxon::String("U")))
            return false;
        if (!SetString(YSTR, maxon::String("V")))
            return false;
        if (!SetString(ZSTR, maxon::String("W")))
            return false;
    }
    
    return true;
    
}

Bool ObjectResizeDialog::UpdateUI_()
{
    ActivateField_(false);
    SetUItxt_(0);
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return false;
    AutoAlloc<AtomArray> selection;
    if (!selection)
        return false;
    doc->GetActiveObjects(*selection, GETACTIVEOBJECTFLAGS::CHILDREN);
    if (selection->GetCount() ==0)
        return false;

    if (!CheckObjectType_(selection)) {
        SetUIValue_(0.0, 0.0, 0.0);
        return false;
    }
    
    if (ToPoly(selection->GetIndex(0))->IsInstanceOf(Onull))
    {
        Vector scaleRatio = ToPoly(selection->GetIndex(0))->GetRelScale();
        SetUIValue_(scaleRatio.x, scaleRatio.y, scaleRatio.z,false,FORMAT_FLOAT);
        ActivateField_(true);
        return true;
    }
    
    
    Int32 docMode = doc->GetMode();
    if ((docMode == Muvpolygons) ||
        (docMode == Muvpoints) )
        
    {
        SetUItxt_(1);
        if (selection->GetCount() > 1)
            SetUIValue_(-1.0, -1.0, -1.0, true,FORMAT_FLOAT);
        else
        {
            Vector sizebb, centerbb;
            GetUVWSelectedSize(selection->GetIndex(0),sizebb,centerbb);
            SetUIValue_(sizebb.x, sizebb.y, 0.0,false,FORMAT_FLOAT);

            ActivateField_(true);
        }
    }
    
    else if ((docMode == Mpoints) ||
        (docMode == Mpolygons) ||
        (docMode == Medges)
        )
    {

        if (selection->GetCount() > 1 )
            SetUIValue_(-1.0, -1.0, -1.0, true);
        else
        {
            Vector actualSize (Vector(0));
            actualSize = GetSelectionSize_(selection->GetIndex(0), docMode);
            
            ActivateField_(true);
            SetUIValue_(actualSize.x, actualSize.y, actualSize.z);
        }
    }
    
    
    else if ((docMode = Mmodel) || (docMode == Mobject))
    {
        // all selected object have good type
        ActivateField_(true);
        if (selection->GetCount() > 1 )
        {
            SetUIValue_(-1.0, -1.0, -1.0, true);
        }
        else
        {
            BaseObject *op = (BaseObject*)selection->GetIndex(0);
            if (!op)
                return false;
            Vector actualSize = GetObjectSize_(op);
            SetUIValue_(actualSize.x, actualSize.y, actualSize.z);
        }
        
    }
    
    return true;
}

Vector ObjectResizeDialog::GetObjectSize_(BaseObject *op)
{
    
    
    
    if (!op)
        return Vector(0);
    Matrix pmg = op->GetUpMg();
    Vector size = Vector(1);
    if (op->IsInstanceOf(Onull))
        return op->GetAbsScale();
    
    size = op->GetRad() * 2.0 * op->GetAbsScale();
	
    size.x *= pmg.sqmat.v1.GetLength();
    size.y *= pmg.sqmat.v2.GetLength();
    size.z *= pmg.sqmat.v3.GetLength();
    
    
    return size;
}
Bool ObjectResizeDialog::SetUIValue_(Float sizeX, Float sizeY, Float sizeZ, Bool tristate , Int32 format)
{
    
    if (!SetFloat(ID_VSIZEX, sizeX,-1.0e18,1.0e18,1.0,format,0.0,0.0, false,tristate))
        return false;
    if (!SetFloat(ID_VSIZEY, sizeY,-1.0e18,1.0e18,1.0,format,0.0,0.0, false,tristate))
        return false;
    if (!SetFloat(ID_VSIZEZ, sizeZ,-1.0e18,1.0e18,1.0,format,0.0,0.0, false,tristate))
        return false;
    return true;
}

Bool ObjectResizeDialog::ScaleObject_(BaseObject *op, Vector &ratio)
{
    BaseDocument *doc = GetActiveDocument();
    if (!doc)
        return false;
    if (CompareFloatTolerant(ratio.x, 0.0))
        return false;
    if (CompareFloatTolerant(ratio.y, 0.0))
        return false;
    if (CompareFloatTolerant(ratio.z, 0.0))
        return false;
    
    if (doc->GetMode()==Mobject || op->IsInstanceOf(Onull))
    {
        ratio *= op->GetRelScale();
        if (op->IsInstanceOf(Onull))
            op->SetRelScale(ratio*2.0);
        else
            op->SetRelScale(ratio);
    }
    
    else
    {
        Vector* paddr = ToPoly(op)->GetPointW();
        if (!paddr)
            return false;
        
        for (Int32 i = 0; i < ToPoly(op)->GetPointCount(); i++, paddr++)
        {
            *paddr *= ratio;
        }
        
        if ((op->GetInfo() & OBJECT_ISSPLINE) == OBJECT_ISSPLINE)
        {
            SplineObject* opSpline = (SplineObject*)op;
            if (opSpline->GetInterpolationType() == SPLINETYPE::BEZIER)
            {
                Tangent *top = opSpline->GetTangentW();
                for (Int32 i = 0 ; i < opSpline->GetTangentCount(); i++, top++)
                {
                    top->vl *= ratio;
                    top->vr *= ratio;
                }
            }
        }
    }
    op->Message(MSG_UPDATE);
    
    return true;
}


Bool ObjectResizeDialog::ModifyScaleObject_()
{
    BaseDocument *doc = GetActiveDocument();
    if (!doc)
        return false;
    AutoAlloc<AtomArray> selection;
    if (!selection)
        return false;
    doc->GetActiveObjects(*selection, GETACTIVEOBJECTFLAGS::CHILDREN);
    if (selection->GetCount()==0)
        return false;
    if (!CheckObjectType_(selection))
        return false;
    
    
    if (selection->GetCount()>1)
    {
        Vector ratioUI = Vector(0.0);
        GetFloat(ID_VSIZEX, ratioUI.x);
        GetFloat(ID_VSIZEY, ratioUI.y);
        GetFloat(ID_VSIZEZ, ratioUI.z);
        doc->StartUndo();

        
        Bool bLockX;
        GetBool(LOCKX, bLockX);
        
        Bool bLockY;
        GetBool(LOCKY, bLockY);
        
        Bool bLockZ;
        GetBool(LOCKZ, bLockZ);
        
        
        for (Int32 i = 0; i < selection->GetCount(); i++)
        {
            BaseObject* op = (BaseObject*)selection->GetIndex(i);
            Vector ratio = Vector(1.0);
            Vector currentSize = GetObjectSize_(op);
            
            if (!CompareFloatTolerant(ratioUI.x, 0.0))
            {
                ratio.x = ratioUI.x / currentSize.x;
                if (bLockY)
                    ratio.y = ratio.x;
                if (bLockZ)
                    ratio.z = ratio.x;
                doc->AddUndo(UNDOTYPE::CHANGE, op);
                ScaleObject_(op, ratio);
            }

            else if (!CompareFloatTolerant(ratioUI.y, 0.0))
            {
                ratio.y = ratioUI.y / currentSize.y;
                if (bLockX)
                    ratio.x = ratio.y;
                if (bLockZ)
                    ratio.z = ratio.y;
                
                doc->AddUndo(UNDOTYPE::CHANGE, op);
                ScaleObject_(op, ratio);
            }

            else if (!CompareFloatTolerant(ratioUI.z, 0.0))
            {
                ratio.z = ratioUI.z / currentSize.z;
                if (bLockY)
                    ratio.y = ratio.z;
                if (bLockX)
                    ratio.x = ratio.z;

                doc->AddUndo(UNDOTYPE::CHANGE, op);
                ScaleObject_(op, ratio);
            }
            
        }
        
        doc->EndUndo();
        
    }
    else
    {
        BaseObject *op = (BaseObject*)selection->GetIndex(0);
        doc->StartUndo();
        Vector ratio  = GetRatio(GetObjectSize_(op));
        doc->AddUndo(UNDOTYPE::CHANGE, op);
        if (op->IsInstanceOf(Onull))
            op->SetAbsScale(op->GetAbsScale() * ratio);
        else
            ScaleObject_(op, ratio);
        doc->EndUndo();
        
    }
    
    
    
    return true;
}

Vector ObjectResizeDialog::GetSelectionSize_(C4DAtom* op, Int32 mode)
{
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return Vector(-1);

    LMinMax bb;
    bb.Init();
    PolygonObject* obj = (PolygonObject*)op;
    if (!obj)
        return Vector(-1);
    BaseSelect* bs;
    const Vector* paddr = obj->GetPointR();
    
    BaseContainer bc = doc->GetData(DOCUMENTSETTINGS::GENERAL);
    Bool stateW = bc.GetBool(DOCUMENT_STATEW);
    BaseDraw *bd = doc->GetActiveBaseDraw();
    if (!bd)
        return Vector(-1);
    Matrix mwp = GetWorkplaneMatrix(doc, nullptr);
    
    
    const Matrix objMg = ToPoly(op)->GetMg();
    
    
    if (mode == Mpoints)
    {
        bs = obj->GetPointS();
        for (Int32 i = 0 ; i < obj->GetPointCount(); i++, paddr++)
            if (bs->IsSelected(i))
            {
                if (stateW)
                    bb.AddPoint( ~mwp * objMg * *paddr ) ;
                else
                    bb.AddPoint( *paddr) ;
            }
    }
    else if (mode == Mpolygons)
    {
        bs = obj->GetPolygonS();
        const CPolygon* pcaddr = obj->GetPolygonR();
        
        
        for (Int32 i = 0; i < obj->GetPolygonCount(); i++ , pcaddr++)
            if (bs->IsSelected(i))
                for (Int32 j= 0; j < 4; j++)
                {
                    if (stateW)
                        bb.AddPoint(~mwp * objMg * paddr[  (*pcaddr)[j] ]  );
                    else
                        bb.AddPoint(paddr[  (*pcaddr)[j] ]  );
                }
        
    }
    else if (mode == Medges)
    {
        bs = obj->GetEdgeS();
        const CPolygon* pcaddr = obj->GetPolygonR();
        
        for (Int32 i = 0 ; i < obj->GetPolygonCount() * 4; i ++)
        {
            if (bs->IsSelected(i))
            {
                const CPolygon &poly = pcaddr[ (Int32)( i / 4)];
                Int32 edge = i % 4;
                if (edge == 0)
                {
                    if (stateW)
                        bb.AddPoints(~mwp *objMg * paddr[poly.a],~mwp *objMg * paddr[poly.b]);
                    else
                        bb.AddPoints(paddr[poly.a],paddr[poly.b]);
                }
                if (edge == 1)
                {
                    if (stateW)
                        bb.AddPoints(~mwp *objMg * paddr[poly.b],~mwp *objMg * paddr[poly.c]);
                    else
                        bb.AddPoints(paddr[poly.b],paddr[poly.c]);
                }
                if (edge == 2)
                {
                    if (stateW)
                        bb.AddPoints(~mwp *objMg * paddr[poly.c],~mwp *objMg * paddr[poly.d]);
                    else
                        bb.AddPoints(paddr[poly.c],paddr[poly.d]);
                }
                if (edge == 3)
                {
                    if (stateW)
                        bb.AddPoints(~mwp *objMg * paddr[poly.d],~mwp *objMg * paddr[poly.a]);
                    else
                        bb.AddPoints(paddr[poly.d],paddr[poly.a]);
                }
                
            }
            
        }

        
    }
    Vector size = Vector(1);
    
    
    
    if (stateW)
    {
        size = bb.GetRad() * 2.0;
    }
    else{
        Matrix pmg = obj->GetUpMg();
        size = bb.GetRad() * 2.0 * obj->GetAbsScale();
        size.x *= pmg.sqmat.v1.GetLength();
        size.y *= pmg.sqmat.v2.GetLength();
        size.z *= pmg.sqmat.v3.GetLength();
    }
    

    return size;
}

Vector ObjectResizeDialog::GetRatio(Vector actualSize)
{
    //avoid divide by zero
    Vector ratio = Vector(1.0);
    Float ratioG = 1.0;
    // avoid divide by 0
    if (CompareFloatTolerant(actualSize.x, 0.0))
        ratio.x = 1.0;
    else
    {
        GetFloat(ID_VSIZEX, ratio.x);
        ratio.x /= actualSize.x;
    }
    
    if (CompareFloatTolerant(actualSize.y, 0.0))
        ratio.y = 1.0;
    else
    {
        GetFloat(ID_VSIZEY, ratio.y);
        ratio.y /= actualSize.y;
        
    }
    if (CompareFloatTolerant(actualSize.z, 0.0))
        ratio.z = 1.0;
    else
    {
        GetFloat(ID_VSIZEZ, ratio.z);
        ratio.z /= actualSize.z;
        
    }
        if (!CompareFloatTolerant(ratio.x, 1.0))
            ratioG = ratio.x;
        else if (!CompareFloatTolerant(ratio.y, 1.0))
            ratioG = ratio.y;
        else if (!CompareFloatTolerant(ratio.z, 1.0))
            ratioG = ratio.z;
    
    Vector res(1.0);
    
    Bool bLockX;
    GetBool(LOCKX, bLockX);
     if (bLockX)
         res.x = ratioG;
    
    Bool bLockY;
    GetBool(LOCKY, bLockY);
    if (bLockY)
        res.y = ratioG;
    
    Bool bLockZ;
    GetBool(LOCKZ, bLockZ);
    if (bLockZ)
        res.z = ratioG;
    
    
    return res;

}

Bool ObjectResizeDialog::ModifyScaleSelection_()
{
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return false;
    AutoAlloc<AtomArray> selection;
    if (!selection)
        return false;
    doc->GetActiveObjects(*selection, GETACTIVEOBJECTFLAGS::CHILDREN);
    if (selection->GetCount() ==0)
        return false;
    
    BaseContainer bc = doc->GetData(DOCUMENTSETTINGS::GENERAL);
    
    
    BaseDraw *bd = doc->GetActiveBaseDraw();
    if (!bd)
        return false;
    
    
    
    if (selection->GetCount() ==1)
    {
        PolygonObject *op  = (PolygonObject*)selection->GetIndex(0);

        
        Matrix mdaxis = Matrix();

        mdaxis = op->GetModelingAxis(doc);

        
        Vector *paddr = op->GetPointW();
        
        BaseSelect *bs = op->GetPointS();
        if (doc->GetMode() == Mpolygons)
        {
            bs->DeselectAll();
            BaseSelect *pbc = op->GetPolygonS();
            const CPolygon* pcaddr = op->GetPolygonR();
            for (Int32 i = 0; i< op->GetPolygonCount(); i ++ , pcaddr++)
            {
                if (pbc->IsSelected(i))
                {
                    for (Int32 j = 0; j < 4 ; j++)
                        bs->Select( (*pcaddr)[j]);
                }
            }
        }
        if (doc->GetMode() == Medges)
        {
            bs->DeselectAll();
            BaseSelect *ebs = op->GetEdgeS();
            const CPolygon* pcaddr = op->GetPolygonR();
            
            for (Int32 i = 0 ; i < op->GetPolygonCount() * 4; i ++)
            {
                if (ebs->IsSelected(i))
                {
                    const CPolygon &poly = pcaddr[ (Int32)( i / 4)];
                    Int32 edge = i % 4;
                    if (edge == 0)
                    {
                        bs->Select(poly.a);
                        bs->Select(poly.b);
                    }

                    if (edge == 1)
                    {
                        bs->Select(poly.b);
                        bs->Select(poly.c);
                    }

                    
                    if (edge == 2)
                    {
                        bs->Select(poly.c);
                        bs->Select(poly.d);
                    }

                    if (edge == 3)
                    {
                        bs->Select(poly.d);
                        bs->Select(poly.a);
                    }
                    
                }
                
            }

        }
        
        
        const Matrix mg = op->GetMg();

        
        
        Vector ratio = GetRatio( GetSelectionSize_(selection->GetIndex(0), doc->GetMode()));
        
        
        
        doc->StartUndo();
        doc->AddUndo(UNDOTYPE::CHANGE, op);
        for (Int32 i  = 0; i < op->GetPointCount(); i++, paddr++)
            if (bs->IsSelected(i))
            {
                    *paddr = ~mg * mdaxis * (ratio * (~mdaxis * mg * *paddr));
            }
        
        if ((op->GetInfo() & OBJECT_ISSPLINE) == OBJECT_ISSPLINE)
        {
            SplineObject* opSpline = (SplineObject*)op;
            if (opSpline->GetInterpolationType() == SPLINETYPE::BEZIER)
            {
                Tangent *top = opSpline->GetTangentW();
                for (Int32 i = 0 ; i < opSpline->GetTangentCount(); i++, top++)
                {
                    if (bs->IsSelected(i))
                    {
                        top->vl *= ratio;
                        top->vr *= ratio;
                    }
                }
            }
        }
        
        doc->EndUndo();
        op->Message(MSG_UPDATE);
        
        
        
    }
    
    return true;
}

Bool ObjectResizeDialog::ScaleUVWs_()
{
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return false;
    // CallCommand(170103) open the texture view.
    
    AutoAlloc<AtomArray> selection;
    if (!selection)
        return false;
    doc->GetActiveObjects(*selection, GETACTIVEOBJECTFLAGS::CHILDREN);
    if (selection->GetCount() ==0)
        return false;
    
    PolygonObject* op = (PolygonObject*) selection->GetIndex(0);
    
    if (selection->GetCount() == 1)
    {
        TempUVHandle* handle = GetActiveUVSet(doc, GETACTIVEUVSET_ALL);
        if (handle)
        {
            UVWStruct* uvwAddr = handle->GetUVW();
            if (!uvwAddr)
                return false;
            Vector sizebb;
            Matrix center = Matrix();
            GetUVWSelectedSize(selection->GetIndex(0), sizebb, center.off);
            Vector ratio = Vector(1.0);
            
            
            ratio = GetRatio(sizebb);
            
            
            
            if (doc->GetMode()==Muvpoints)
            {
                BaseSelect* bsp = handle->GetUVPointSel();
                for (Int32 i = 0;  i < op->GetPolygonCount() * 4; i++)
                    if (bsp->IsSelected(i))
                        uvwAddr[i/4][i % 4] =  center * ( ~center * uvwAddr[i/4][i % 4] *  ratio);
                
            }
            else if (doc->GetMode()==Muvpolygons)
            {
                BaseSelect* bs = handle->GetPolySel();
                for (Int32 i = 0 ; i < op->GetPolygonCount();i++)
                    if (bs->IsSelected(i))
                    {
                        uvwAddr[i].a = center * (~center * uvwAddr[i].a * ratio);
                        uvwAddr[i].b = center * (~center * uvwAddr[i].b * ratio);
                        uvwAddr[i].c = center * (~center * uvwAddr[i].c * ratio);
                        uvwAddr[i].d = center * (~center * uvwAddr[i].d * ratio);
                    }

                
            }
            
            
            handle->SetUVW(uvwAddr);
            
        }
      
        
        
        FreeActiveUVSet(handle);
    }
    
    op->Message(MSG_UPDATE);
    
    return false;
}


Bool ObjectResizeDialog::Modification_()
{
    BaseDocument* doc = GetActiveDocument();
    if (!doc)
        return false;
    Int32 docMode = doc->GetMode();
    switch (docMode) {
        case Mobject:
        case Mmodel:
            ModifyScaleObject_();
            break;
        case Mpoints:
        case Mpolygons:
        case Medges:
            ModifyScaleSelection_();
            break;
        case Muvpolygons:
        case Muvpoints:
            ScaleUVWs_();
            break;
    }
    
    EventAdd();
    
    return true;
}



Bool ObjectResizeDialog::InitValues()
{

    SetUIValue_(0.0,0.0,0.0);
    SetBool(LOCKX, true);
    SetBool(LOCKY, true);
    SetBool(LOCKZ, true);
    ActivateField_(false);
    UpdateUI_();
    return true;
}

Bool ObjectResizeDialog::CreateLayout()
{
    SetTitle(GeLoadString(DIALOG_TITLE));
    GroupBegin(SIZEGROUP, BFH_SCALEFIT, 3, 1, maxon::String(""), 0);
        GroupBorderSpace(3,1,1,3);
        GroupBegin(XGROUP, BFH_SCALEFIT, 3, 1, maxon::String(""), 0);
        AddStaticText(XSTR, BFH_FIT, SizeChr(13), SizeChr(13), maxon::String("X"), BORDER_NONE);
        AddCheckbox(LOCKX, BFH_FIT,SizeChr(1) , SizeChr(1), maxon::String(""));
        AddEditNumber(ID_VSIZEX, BFH_SCALEFIT);
        GroupEnd();
    
        GroupBegin(YGROUP, BFH_SCALEFIT, 3, 1, maxon::String(""), 0);
        AddStaticText(YSTR, BFH_FIT, SizeChr(13), SizeChr(13), maxon::String("Y"), BORDER_NONE);
        AddCheckbox(LOCKY, BFH_FIT,SizeChr(1) , SizeChr(1), maxon::String(""));
        AddEditNumber(ID_VSIZEY, BFH_SCALEFIT);
        GroupEnd();
    
        GroupBegin(ZGROUP, BFH_SCALEFIT, 3, 1, maxon::String(""), 0);
        AddStaticText(ZSTR, BFH_FIT, SizeChr(13), SizeChr(13), maxon::String("Z"), BORDER_NONE);
        AddCheckbox(LOCKZ, BFH_FIT,SizeChr(1) , SizeChr(1), maxon::String(""));
        AddEditNumber(ID_VSIZEZ, BFH_SCALEFIT);
        GroupEnd();
    
    GroupEnd();
    
    return GeDialog::CreateLayout();
}

Bool ObjectResizeDialog::CoreMessage(Int32 id, const BaseContainer &msg)
{
    
    if (id == EVMSG_CHANGE)
        UpdateUI_();
    return GeDialog::CoreMessage(id, msg);
}

Bool ObjectResizeDialog::Command(Int32 id, const BaseContainer &msg)
{
    
    if (id == ID_VSIZEX || id==ID_VSIZEY || id==ID_VSIZEZ )
        Modification_();
    if (id == LOCKX || id==LOCKY || id==LOCKZ )
        UpdateSizeField_();
    
    return GeDialog::Command(id, msg);
}




Bool ObjectResizeCommand::Execute(BaseDocument* doc, GeDialog* parentManager)
{
	return dlg_.Open(DLG_TYPE::ASYNC, ID_OBJECT_RESIZE, -1, -1, 400, 50);
}

Bool ObjectResizeCommand::RestoreLayout(void *secret)
{
    return dlg_.RestoreLayout(ID_OBJECT_RESIZE, 0, secret);
    
}




Bool RegisterObjectResize(void);

Bool RegisterObjectResize() {
    Filename iconPath = GeGetPluginPath();
    iconPath += "res";  iconPath += "icons";  iconPath +="hom-resize.tif";
    AutoAlloc<BaseBitmap> icon;
    icon->Init(iconPath.GetString());

    
      return RegisterCommandPlugin(ID_OBJECT_RESIZE, GeLoadString(DIALOG_TITLE), 0, icon, GeLoadString(ID_HELP), ObjectResizeCommand::Alloc());
    
}

