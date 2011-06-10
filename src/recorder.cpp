/*

Recorder.cpp : Defines the entry point for the console application.

*/

#include "stdafx.h"
#define XMLCONFIG_FILENAME "./recorder_config.xml"
#define RECORDER_VERSION "v1.0.0"

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
			printf("OpenNI Recorder %s by Dale Phurrough\nUsage: record.exe secondsToRecord [xmlconfigfilename]\n", RECORDER_VERSION);
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

