//
//  main.cpp
//  GeoDesicVoxel
//
//  Created by Manuel MAGALHAES on 08/09/2015.
//  Copyright (c) 2015 MAXON Computer GmbH. All rights reserved.
//

#include <stdio.h>
#include "c4d.h"
#include "main.h"

Bool PluginStart(void)
{
    
    if (!RegisterObjectResize())
        return false;
    return true;
}

void PluginEnd(void)
{
    
    
    
}

Bool PluginMessage(Int32 id, void* data)
{
    
    switch (id)
    {
        case C4DPL_INIT_SYS:
            if (!resource.Init())
                return false; // don't start the plugin without resource
            break;
        case C4DMSG_PRIORITY:
            //react to this message to set a plugin priority (to determine in which order plugins are initialized or loaded
            //SetPluginPriority(data, mypriority);
            break;
        case C4DPL_BUILDMENU:
            //react to this message to dynamically enhance the menu
            //EnhanceMainMenu();
            break;
        case C4DPL_COMMANDLINEARGS:
            //sample implementation of command line rendering:
            //CommandLineRendering((C4DPL_CommandLineArgs*)data);
            break;
            
        case C4DPL_EDITIMAGE:
            break;
            
    }
    
    
    return false;
    
}
