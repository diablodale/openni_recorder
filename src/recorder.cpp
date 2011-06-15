/****************************************************************************
	OpenNI_Recorder
	Copyright (C) 2011 Dale Phurrough

	This file is part of OpenNI_Recorder.

    OpenNI_Recorder is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenNI_Recorder is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenNI_Recorder.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include "stdafx.h"
#define XMLCONFIG_FILENAME "./recorder_config.xml"
#define RECORDER_VERSION "v1.0.1"

int _tmain(int argc, _TCHAR* argv[])
{
	XnStatus nRetVal = XN_STATUS_OK;
	Context g_context;
	DepthGenerator g_depth;
	ImageGenerator g_image;
	DepthMetaData g_depthMD;
	ImageMetaData g_imageMD;
		
	// Initialize g_context object
	//nRetVal = g_context.Init();
	EnumerationErrors errors;


	char xml_filename[1024];
	int iSecondsToRecord;

	switch(argc)
	{
		case 1:
			printf("OpenNI Recorder %s, Copyright (c) 2011 Dale Phurrough\nThis program comes with ABSOLUTELY NO WARRANTY.\n", RECORDER_VERSION);
			printf("Licensed under the GNU General Public License v3.0 (GPLv3) available at http://www.gnu.org/licenses/gpl-3.0.html\n");
			printf("\nUsage: record.exe secondsToRecord [xmlconfigfilename]\n");
			exit(0);
		case 2:
			// printf("Reading from default config filë\n");
			nRetVal = g_context.InitFromXmlFile(XMLCONFIG_FILENAME, &errors);
			break;
		case 3:
			// wprintf(L"file arg:%s\n", argv[2]);
			WideCharToMultiByte(CP_ACP, 0, argv[2], -1, xml_filename, 1024, NULL, NULL);
			// printf("convertedfilename:%s\n", xml_filename);
			nRetVal = g_context.InitFromXmlFile(xml_filename, &errors);
			break;
		default:
			printf("ERROR: too many arguments\n");
			return(1);
	}
	iSecondsToRecord = _ttoi(argv[1]);
	// printf("Seconds to record: %i\n", iSecondsToRecord);

	if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
	{
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		printf("ERROR: XML config initialization failed: %s\n", strError);
		return (nRetVal);
	}
	else if (nRetVal != XN_STATUS_OK)
	{
		printf("ERROR: XML config initialization open failed: %s\n", xnGetStatusString(nRetVal));
		return (nRetVal);
	}
	
	nRetVal = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("No DEPTH node created/found: %s\n", xnGetStatusString(nRetVal));
	}
	nRetVal = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);
	if (nRetVal != XN_STATUS_OK)
	{
		printf("No IMAGE node created/found: %s\n", xnGetStatusString(nRetVal));
	}

	if (!g_depth && !g_image)
	{
		printf("ERROR: No GENERATORS, therefore nothing to record\n");
		return (0);
	}

	
	
	if (g_depth)
	{
		XnUInt32 numDepthMapModes = g_depth.GetSupportedMapOutputModesCount();
		XnMapOutputMode *depthMapModes;
		depthMapModes = (XnMapOutputMode *)malloc(sizeof(XnMapOutputMode) * numDepthMapModes);
		g_depth.GetSupportedMapOutputModes(depthMapModes, numDepthMapModes);
		printf("== %lu Depth modes avail==\n", numDepthMapModes);
		for (XnUInt32 i=0; i<numDepthMapModes; i++)
		{
			printf("FPS=%lu X=%lu Y=%lu  Z=%u\n", depthMapModes[i].nFPS, depthMapModes[i].nXRes, depthMapModes[i].nYRes, g_depth.GetDeviceMaxDepth());
		}
		g_depth.GetMetaData(g_depthMD);
	}
	if (g_image)
	{
		XnUInt32 numImageMapModes = g_image.GetSupportedMapOutputModesCount();
		XnMapOutputMode *imageMapModes;
		imageMapModes = (XnMapOutputMode *)malloc(sizeof(XnMapOutputMode) * numImageMapModes);
		g_image.GetSupportedMapOutputModes(imageMapModes, numImageMapModes);
		printf("== %lu Image modes avail==\n", numImageMapModes);
		for (XnUInt32 i=0; i<numImageMapModes; i++)
		{
			printf("FPS=%lu X=%lu Y=%lu\n", imageMapModes[i].nFPS, imageMapModes[i].nXRes, imageMapModes[i].nYRes);
		}	
		g_image.GetMetaData(g_imageMD);
	}
	
	printf("==Current active modes==\n");
	if (g_depth) printf("DepthMD FPS=%lu X=%lu Y=%lu  Z=%u\n", g_depthMD.FPS(), g_depthMD.FullXRes(), g_depthMD.FullYRes(), g_depthMD.ZRes());
	if (g_image) printf("ImageMD FPS=%lu X=%lu Y=%lu\n", g_imageMD.FPS(), g_imageMD.XRes(), g_imageMD.YRes());
	
	// Main loop
	unsigned int counter = g_depthMD.FPS() * iSecondsToRecord;
	while (counter)
	{
		printf("Seconds left: %i-%i\n", counter / g_depthMD.FPS(), counter % g_depthMD.FPS());
		
		// Wait for new data to be available
		nRetVal = g_context.WaitOneUpdateAll(g_depth);
		if (nRetVal != XN_STATUS_OK)
		{
			printf("Failed updating data: %s\n", xnGetStatusString(nRetVal));
			continue;
		}
	
		// Take current depth map THIS is what was written in the demo
		const XnDepthPixel* pDepthMap = g_depth.GetDepthMap();
		
		// THIS is what was written on the openNI google group saying The SceneAnalysis sample passes the depth meta data to a DrawDepthMap function (implemented in SceneDrawer.cpp). 
		// n::DepthMetaData depthMD; 
		// g_DepthGenerator.GetMetaData(depthMD); 
		// const XnDepthPixel* pDepth = depthMD.Data(); 
		
		// TODO: process depth map
		
		counter -= 1;
	}
	
	// Clean-up
	printf("Beginning to exit\n");
	g_context.Shutdown();
	printf("Exiting gracefully\n");

}

