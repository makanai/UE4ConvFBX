/****************************************************************************************

   Copyright (C) 2013 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This example illustrates how to detect if a scene is password 
// protected, import and browse the scene to access node and animation 
// information. It displays the content of the FBX file which name is 
// passed as program argument. You can try it with the various FBX files 
// output by the export examples.
//
/////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"
#include "DisplayCommon.h"
#include "DisplayHierarchy.h"
#include "DisplayAnimation.h"
#include "DisplayMarker.h"
#include "DisplaySkeleton.h"
#include "DisplayMesh.h"
#include "DisplayNurb.h"
#include "DisplayPatch.h"
#include "DisplayLodGroup.h"
#include "DisplayCamera.h"
#include "DisplayLight.h"
#include "DisplayGlobalSettings.h"
#include "DisplayPose.h"
#include "DisplayPivotsAndLimits.h"
#include "DisplayUserProperties.h"
#include "DisplayGenericInfo.h"

// Local function prototypes.
void DisplayContent(FbxScene* pScene);
void DisplayContent(FbxNode* pNode);
void DisplayTarget(FbxNode* pNode);
void DisplayTransformPropagation(FbxNode* pNode);
void DisplayGeometricTransform(FbxNode* pNode);
void DisplayMetaData(FbxScene* pScene);

static bool gVerbose = true;

#include <string>
#include <vector>
struct XmlTexture
{
	std::string	filename;
};
std::vector<XmlTexture>	textureList;

struct XmlMaterial
{
	XmlMaterial()
	:textureID(0){

	}
	std::string	name;
	int	textureID;
};
std::vector<XmlMaterial>	materialList;

#include <iostream>
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
void XmlParsing(const char* name)
{
	std::ifstream ifs( name );
	if (!ifs){
		return;
	}
	std::string	data;
	while(!ifs.fail()){
		std::string	temp;
		ifs >> temp;
		data += temp;
	}

	rapidxml::xml_document<> doc;
	try {
		doc.parse<0>((char*)data.c_str());
	}
	catch(rapidxml::parse_error& err) {
		std::cout << err.what() << " " << err.where<char*>();
		return;
	}

	for (rapidxml::xml_node<> *node  = doc.first_node(); node;node = node->next_sibling()) {
		for (rapidxml::xml_node<> *child  = node->first_node(); child;child = child->next_sibling()) {

			if (std::string("textureList").compare(child->name())==0) {

				for (rapidxml::xml_node<> *sub = child->first_node();sub; sub= sub->next_sibling()) {
					if (std::string("Texture").compare(sub->name()) )
						continue;

					textureList.push_back(XmlTexture());
					for (rapidxml::xml_node<> *subsub = sub->first_node();subsub; subsub= subsub->next_sibling()) {
						if (std::string("fileName").compare(subsub->name())){
							continue;
						}
						textureList.rbegin()->filename = subsub->value();
					}
				}
			}

			if (std::string("materialList").compare(child->name())==0) {
				for (rapidxml::xml_node<> *sub = child->first_node();sub; sub= sub->next_sibling()) {

					if (std::string("Material").compare(sub->name()))
						continue;

					materialList.push_back(XmlMaterial());

					for (rapidxml::xml_node<> *subsub = sub->first_node();subsub; subsub= subsub->next_sibling()) {
						if (std::string("nameJp").compare(subsub->name())==0){
							materialList.rbegin()->name = subsub->value();
						}
						if (std::string("textureID").compare(subsub->name())==0){
							materialList.rbegin()->textureID = atoi(subsub->value());
						}
					}
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;
	bool lResult;

	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);
	// Load the scene.

	// The example can take a FBX file as an argument.
	FbxString lFilePath("");
	for( int i = 1, c = argc; i < c; ++i )
	{
		if( FbxString(argv[i]) == "-test" ) gVerbose = false;
		else if( lFilePath.IsEmpty() ) lFilePath = argv[i];
	}

	std::string	fileBody = lFilePath.Buffer();
	int	index = fileBody.rfind('.');
	if (index>=0){
		fileBody = fileBody.substr(0,index);
	}

	std::string	xmlFileName = fileBody ;
	xmlFileName += ".xml";
	XmlParsing(xmlFileName.c_str());


	if( lFilePath.IsEmpty() )
	{
		lResult = false;
		FBXSDK_printf("\n\nUsage: ImportScene <FBX file name>\n\n");
	}
	else
	{
		FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer());
		lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
	}

	if(lResult == false)
	{
		FBXSDK_printf("\n\nAn error occurred while loading the scene...");
	}
	else 
	{
#if 0
		// Display the scene.
		DisplayMetaData(lScene);

		FBXSDK_printf("\n\n---------------------\nGlobal Light Settings\n---------------------\n\n");

		if( gVerbose ) DisplayGlobalLightSettings(&lScene->GetGlobalSettings());

		FBXSDK_printf("\n\n----------------------\nGlobal Camera Settings\n----------------------\n\n");

		if( gVerbose ) DisplayGlobalCameraSettings(&lScene->GetGlobalSettings());

		FBXSDK_printf("\n\n--------------------\nGlobal Time Settings\n--------------------\n\n");

		if( gVerbose ) DisplayGlobalTimeSettings(&lScene->GetGlobalSettings());

		FBXSDK_printf("\n\n---------\nHierarchy\n---------\n\n");

		if( gVerbose ) DisplayHierarchy(lScene);

		FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");

		if( gVerbose ) DisplayContent(lScene);

		FBXSDK_printf("\n\n----\nPose\n----\n\n");

		if( gVerbose ) DisplayPose(lScene);

		FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");

		if( gVerbose ) DisplayAnimation(lScene);

		//now display generic information

		FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
		if( gVerbose ) DisplayGenericInfo(lScene);
#endif
	}

	for (int m=0; m<lScene->GetMaterialCount(); ++m){
		FbxSurfaceMaterial* lMaterial = lScene->GetMaterial(m);
		FbxPropertyT<FbxString> lString = lMaterial->ShadingModel;
		FBXSDK_printf("\n%s", lString.Get().Buffer() );

		//Get the implementation to see if it's a hardware shader.
		const FbxImplementation* lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_HLSL);
		FbxString lImplemenationType = "HLSL";
		if(!lImplementation)
		{
			lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_CGFX);
			lImplemenationType = "CGFX";
		}
		if(lImplementation)	{
		}else{
			if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId)){
				FbxSurfaceLambert* lLambert =  (FbxSurfaceLambert*)lMaterial;
				int count = lLambert->Diffuse.GetSrcObjectCount();

				std::string	materialName = lLambert->GetName();
				FBXSDK_printf("\n%s",materialName.c_str());

				if (count==0){
					if (materialName.find('.')>=0){
						materialName = materialName.substr(materialName.find('.')+1);
					}
					for(std::vector<XmlMaterial>::iterator it=materialList.begin();it!=materialList.end();++it) {
						if (it->name==materialName) {
							FbxFileTexture* lTexture = FbxFileTexture::Create(lScene,"Diffuse Texture");
							lTexture->SetFileName(textureList[it->textureID].filename.c_str());
							lTexture->SetTextureUse(FbxTexture::eStandard);
							lTexture->SetMappingType(FbxTexture::eUV);
							lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
							lTexture->SetSwapUV(false);
							lTexture->SetTranslation(0.0, 0.0);
							lTexture->SetScale(1.0, 1.0);
							lTexture->SetRotation(0.0, 0.0);
							lLambert->Diffuse.ConnectSrcObject(lTexture);
							break;
						}
					}

				}else{
					for (int d=0; d<count; ++d) {
						FbxObject*	obj = lLambert->Diffuse.GetSrcObject(d);
						FBXSDK_printf("\n%s", obj->GetClassId().GetName() );
						if (obj->GetClassId().Is(FbxFileTexture::ClassId)){
							FbxFileTexture* lTexture = (FbxFileTexture*)obj;
							const char* filename = lTexture->GetFileName();
							FbxString newFileName = filename;
							int index = newFileName.ReverseFind('\\');
							if (index>=0) {
								newFileName = newFileName.Right(newFileName.GetLen()-index-1);
								FBXSDK_printf("\n%s",newFileName.Buffer());
							}
							lTexture->SetFileName(newFileName.Buffer());
						}
						FBXSDK_printf("\n%s",obj->GetName());
					}
				}
			}
		}
	}

	FbxExporter* lExporter = FbxExporter::Create(lSdkManager,"");
	std::string exportFileName = fileBody + "_rename.fbx";
	if (lExporter->Initialize(exportFileName.c_str(), -1, lSdkManager->GetIOSettings()) ){
		lExporter->Export(lScene);
	}

	lExporter->Destroy();

	// Destroy all objects created by the FBX SDK.
	DestroySdkObjects(lSdkManager, lResult);

	return 0;
}

void DisplayContent(FbxScene* pScene)
{
	int i;
	FbxNode* lNode = pScene->GetRootNode();

	if(lNode)
	{
		for(i = 0; i < lNode->GetChildCount(); i++)
		{
			DisplayContent(lNode->GetChild(i));
		}
	}
}

void DisplayContent(FbxNode* pNode)
{
	FbxNodeAttribute::EType lAttributeType;
	int i;

	if(pNode->GetNodeAttribute() == NULL)
	{
		FBXSDK_printf("NULL Node Attribute\n\n");
	}
	else
	{
		lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

		switch (lAttributeType)
		{
		case FbxNodeAttribute::eMarker:  
			DisplayMarker(pNode);
			break;

		case FbxNodeAttribute::eSkeleton:  
			DisplaySkeleton(pNode);
			break;

		case FbxNodeAttribute::eMesh:      
			DisplayMesh(pNode);
			break;

		case FbxNodeAttribute::eNurbs:      
			DisplayNurb(pNode);
			break;

		case FbxNodeAttribute::ePatch:     
			DisplayPatch(pNode);
			break;

		case FbxNodeAttribute::eCamera:    
			DisplayCamera(pNode);
			break;

		case FbxNodeAttribute::eLight:     
			DisplayLight(pNode);
			break;

		case FbxNodeAttribute::eLODGroup:
			DisplayLodGroup(pNode);
			break;
		}   
	}

	DisplayUserProperties(pNode);
	DisplayTarget(pNode);
	DisplayPivotsAndLimits(pNode);
	DisplayTransformPropagation(pNode);
	DisplayGeometricTransform(pNode);

	for(i = 0; i < pNode->GetChildCount(); i++)
	{
		DisplayContent(pNode->GetChild(i));
	}
}


void DisplayTarget(FbxNode* pNode)
{
	if(pNode->GetTarget() != NULL)
	{
		DisplayString("    Target Name: ", (char *) pNode->GetTarget()->GetName());
	}
}

void DisplayTransformPropagation(FbxNode* pNode)
{
	FBXSDK_printf("    Transformation Propagation\n");

	// 
	// Rotation Space
	//
	EFbxRotationOrder lRotationOrder;
	pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);

	FBXSDK_printf("        Rotation Space: ");

	switch (lRotationOrder)
	{
	case eEulerXYZ: 
		FBXSDK_printf("Euler XYZ\n");
		break;
	case eEulerXZY:
		FBXSDK_printf("Euler XZY\n");
		break;
	case eEulerYZX:
		FBXSDK_printf("Euler YZX\n");
		break;
	case eEulerYXZ:
		FBXSDK_printf("Euler YXZ\n");
		break;
	case eEulerZXY:
		FBXSDK_printf("Euler ZXY\n");
		break;
	case eEulerZYX:
		FBXSDK_printf("Euler ZYX\n");
		break;
	case eSphericXYZ:
		FBXSDK_printf("Spheric XYZ\n");
		break;
	}

	//
	// Use the Rotation space only for the limits
	// (keep using eEulerXYZ for the rest)
	//
	FBXSDK_printf("        Use the Rotation Space for Limit specification only: %s\n",
		pNode->GetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot) ? "Yes" : "No");


	//
	// Inherit Type
	//
	FbxTransform::EInheritType lInheritType;
	pNode->GetTransformationInheritType(lInheritType);

	FBXSDK_printf("        Transformation Inheritance: ");

	switch (lInheritType)
	{
	case FbxTransform::eInheritRrSs:
		FBXSDK_printf("RrSs\n");
		break;
	case FbxTransform::eInheritRSrs:
		FBXSDK_printf("RSrs\n");
		break;
	case FbxTransform::eInheritRrs:
		FBXSDK_printf("Rrs\n");
		break;
	}
}

void DisplayGeometricTransform(FbxNode* pNode)
{
	FbxVector4 lTmpVector;

	FBXSDK_printf("    Geometric Transformations\n");

	//
	// Translation
	//
	lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FBXSDK_printf("        Translation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

	//
	// Rotation
	//
	lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FBXSDK_printf("        Rotation:    %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

	//
	// Scaling
	//
	lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FBXSDK_printf("        Scaling:     %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
}


void DisplayMetaData(FbxScene* pScene)
{
	FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
	if (sceneInfo)
	{
		FBXSDK_printf("\n\n--------------------\nMeta-Data\n--------------------\n\n");
		FBXSDK_printf("    Title: %s\n", sceneInfo->mTitle.Buffer());
		FBXSDK_printf("    Subject: %s\n", sceneInfo->mSubject.Buffer());
		FBXSDK_printf("    Author: %s\n", sceneInfo->mAuthor.Buffer());
		FBXSDK_printf("    Keywords: %s\n", sceneInfo->mKeywords.Buffer());
		FBXSDK_printf("    Revision: %s\n", sceneInfo->mRevision.Buffer());
		FBXSDK_printf("    Comment: %s\n", sceneInfo->mComment.Buffer());

		FbxThumbnail* thumbnail = sceneInfo->GetSceneThumbnail();
		if (thumbnail)
		{
			FBXSDK_printf("    Thumbnail:\n");

			switch (thumbnail->GetDataFormat())
			{
			case FbxThumbnail::eRGB_24:
				FBXSDK_printf("        Format: RGB\n");
				break;
			case FbxThumbnail::eRGBA_32:
				FBXSDK_printf("        Format: RGBA\n");
				break;
			}

			switch (thumbnail->GetSize())
			{
			case FbxThumbnail::eNotSet:
				FBXSDK_printf("        Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
				break;
			case FbxThumbnail::e64x64:
				FBXSDK_printf("        Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
				break;
			case FbxThumbnail::e128x128:
				FBXSDK_printf("        Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
			}
		}
	}
}

