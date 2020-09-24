#include "StdAfx.h"
#include <dbosnap2.h>
#include <dbobjptr2.h>
#include <dbindex.h>
#include <dbboiler.h>
#include "dbDynamicUCSpe.h"
#include "amodeler.h"
#define CALC_EPSILON  10E-5

/*Utils*/
Acad::ErrorStatus postToDatabase (/*[in]*/AcDbDatabase *pDb /*=NULL*/, AcDbEntity *pEnt, AcDbObjectId &idObj) {
	//----- Purpose:
	//----- Adds an entity to the MODEL_SPACE of the database given in argument.
	//-----   * pDb:   pointer to the databse where to add the entity,
	//-----            if NULL, then the curretn database is used.
	//-----   * pEnt:  pointer to an entity instance.
	//-----   * idObj: it will contain the assign ID to the object if successfully added to the database.
	//----- Note:
	//-----   The entity object is closed while we return from that function. Only the idObj can be used after.
	assert ( pEnt != NULL ) ;

	if ( pDb == NULL )
		pDb =acdbHostApplicationServices ()->workingDatabase () ;
	//----- Get a pointer to the current drawing
	//----- and get the drawing's block table. Open it for read.
	Acad::ErrorStatus es ;
	AcDbBlockTable *pBlockTable ;
	if ( (es =pDb->getBlockTable (pBlockTable, AcDb::kForRead)) == Acad::eOk ) {
		//----- Get the Model Space record and open it for write. This will be the owner of the new line.
		AcDbBlockTableRecord *pSpaceRecord ;
		if ( (es =pBlockTable->getAt (ACDB_MODEL_SPACE, pSpaceRecord, AcDb::kForWrite)) == Acad::eOk ) {
			//----- Append pEnt to Model Space, then close it and the Model Space record.
			if ( (es =pSpaceRecord->appendAcDbEntity (idObj, pEnt)) == Acad::eOk )
				pEnt->close () ;
			pSpaceRecord->close () ;
		}
		pBlockTable->close () ;
	}
	//----- It is good programming practice to return an error status
	return (es) ;
}
int getPrompt(const CString &sPrompt, double &dValue, const bool bGetZeroValue = false)
	{
	TCHAR sMessage[512];
	memset(sMessage, _T('\0'), sizeof(sMessage));
	_tcscpy(sMessage, sPrompt);

	bool bStop = false;
	int nRt;
	while (!bStop)
		{
		if (!bGetZeroValue)
			{
			acedInitGet(RSG_NOZERO | RSG_NONEG, sMessage);
			}
		else
			{
			acedInitGet(RSG_NONEG, sMessage);
			}
		AcGePoint3d ptStart(0, 0, 0);
		nRt = acedGetDist(asDblArray(ptStart), sMessage, &dValue);
		if (nRt == RTNORM)
			{
			bStop = true;
			}
		else if (nRt == RTNONE)
			{
			dValue = 10;
			nRt = RTNORM;
			bStop = true;
			}
		else
			{
			nRt = RTERROR;
			}
		}
	return nRt;
	}


class CMyBox : public AcDb3dSolid
{
public:
	ACRX_DECLARE_MEMBERS(CMyBox);
	CMyBox();
	CMyBox(const double &dWidth,const double dLength,const double dHeight);
	virtual ~CMyBox();
	virtual Acad::ErrorStatus   dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus   dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* pWd);

	void initializeParameter()
	{
		m_dHeight = 0;
		m_dLength = 0;
		m_dWidth = 0;
	}
	bool createSolid(); // create Box
	
	
private:
	double m_dWidth;
	double m_dLength;
	double m_dHeight;
};


///////////////////////////////////////////////
ACRX_DXF_DEFINE_MEMBERS(CMyBox, 
	AcDb3dSolid,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,                       
	AcDbProxyObject::kMergeIgnore,
	CMyBox, 
	MyBox);

//::Make
///////////////////////////////////////////////

CMyBox::CMyBox()
{
	initializeParameter();
}
CMyBox::CMyBox(const double &dWidth,const double dLength,const double dHeight)
{
	m_dWidth = dWidth;
	m_dLength = dLength;
	m_dHeight = dHeight;
}
CMyBox::~CMyBox()
{
};
Acad::ErrorStatus   CMyBox::dwgInFields(AcDbDwgFiler* filer)
{
	assertWriteEnabled();
	filer->readDouble(&m_dLength);													
	filer->readDouble(&m_dHeight);													
	filer->readDouble(&m_dWidth);		
	return filer->filerStatus();
}
Acad::ErrorStatus   CMyBox::dwgOutFields(AcDbDwgFiler* filer) const
{
	assertReadEnabled();
	filer->writeDouble(m_dLength);											
	filer->writeDouble(m_dHeight);											
	filer->writeDouble(m_dWidth);				
	return filer->filerStatus();
}

bool CMyBox::createSolid()
{
	// draw Box
	bool bRet = false;
	AcDbLine *pLine1 = new AcDbLine(AcGePoint3d(0,0,0),AcGePoint3d(m_dWidth,0,0));
	AcDbLine *pLine2 = new AcDbLine(AcGePoint3d(m_dWidth,0,0),AcGePoint3d(m_dWidth,m_dLength,0));
	AcDbLine *pLine3 = new AcDbLine(AcGePoint3d(m_dWidth,m_dLength,0),AcGePoint3d(0,m_dLength,0));
	AcDbLine *pLine4 = new AcDbLine(AcGePoint3d(0,m_dLength,0),AcGePoint3d(0,0,0));

	AcDbVoidPtrArray regions,curveSegments;		
	curveSegments.append(pLine1);		
	curveSegments.append(pLine2);
	curveSegments.append(pLine3);
	curveSegments.append(pLine4);

	// solid
	if(AcDbRegion::createFromCurves(curveSegments,regions)== Acad::eOk)
	{
		AcDbRegion *pRegions = NULL;
		pRegions = (AcDbRegion*)regions[0];
		if(pRegions)
		{					
			AcDbLine *pPath = new AcDbLine(AcGePoint3d(0,0,0),AcGePoint3d(0,0,m_dHeight));
			if(this->extrudeAlongPath(pRegions,pPath,0) == Acad::eOk)
			{				
				bRet = true;
			}			
			delete pRegions; 
			pRegions = NULL;
			delete pPath;
			pPath = NULL;				
		}	
	}
	delete pLine1; pLine1 = NULL;
	delete pLine2; pLine2 = NULL;
	delete pLine3; pLine3 = NULL;
	delete pLine4; pLine4 = NULL;

	return bRet;
}
Adesk::Boolean CMyBox::subWorldDraw(AcGiWorldDraw* pWd)
{
	assertReadEnabled();
	Adesk::Boolean bRet = Adesk::kTrue;
	if (m_dHeight <= CALC_EPSILON)
	{

		// draw Rectang
		AcGePoint3d arPoint[5];
		arPoint[0] = AcGePoint3d(0, 0, 0);
		arPoint[1] = AcGePoint3d(m_dWidth, 0, 0);
		arPoint[2] = AcGePoint3d(m_dWidth, m_dLength, 0);
		arPoint[3] = AcGePoint3d(0, m_dLength, 0);
		arPoint[4] = AcGePoint3d(0, 0, 0);
		pWd->geometry().polyline(5, arPoint);
	}
	else
	{
		AcDb3dSolid::subWorldDraw(pWd);
	}

	return bRet;
}





/*------------------------------------------Dynamic UCS Protocol Extension class-------------------------------------------*/

class AsdkUCSPE : public AcDbDynamicUCSPE
	{
	public:
		//ACRX_DECLARE_MEMBERS(AsdkUCSPE);
		AsdkUCSPE(void);
	public:
		~AsdkUCSPE(void);

		virtual Acad::ErrorStatus getCandidatePlanes(
			AcArray<AcGePlane> &results,
			double &distToEdge,
			double &objWidth,
			double &objHeight,
			AcDbEntity *pEnt,
			const AcDbSubentId &subentId,
			const AcGePlane &viewplane,
			AcDbDynamicUCSPE::Flags flags = kDefault //reserved
		) const;

};


AsdkUCSPE::AsdkUCSPE(void)
	{
	}

AsdkUCSPE::~AsdkUCSPE(void)
	{
	}

Acad::ErrorStatus AsdkUCSPE::getCandidatePlanes(
	AcArray<AcGePlane>& results,
	double& distToEdge,
	double& objWidth,
	double& objHeight,
	AcDbEntity* pEnt,
	const AcDbSubentId& subentId,
	const AcGePlane& viewplane,
	AcDbDynamicUCSPE::Flags flags) const
{
	Acad::ErrorStatus es = Acad::eInvalidInput;
	if (!pEnt)
		return es;

	// Check if User selected custom Solid plane, we are not doing anything here, just to make sure 
	   //Debugger hits us, ideally we are expected to populate the result array 
	   // with one or more AcGePlane objects and return Acad::eOk if successful.

	CMyBox* pSolid = CMyBox::cast(pEnt);
	if (!VERIFY(pSolid != NULL))
		return Acad::eInvalidInput;

	AcDb::SubentType seType = subentId.type();
	if (seType != AcDb::kFaceSubentType)
		return Acad::eWrongSubentityType;

	AcDbFullSubentPath facePath(pEnt->objectId(), subentId);
	std::auto_ptr<AcDbEntity> pFaceSubEnt;
	pFaceSubEnt.reset(pEnt->subentPtr(facePath));
	
	if (pFaceSubEnt.get() && pFaceSubEnt->isKindOf(AcDbRegion::desc()))
	{
		AcDbRegion* reg = AcDbRegion::cast(pFaceSubEnt.get());
		assert(reg != NULL);
		if (!reg)
			return (Acad::eWrongObjectType);
		AcGePlane entPlane;
		AcDb::Planarity kPanarity;
		es = reg->getPlane(entPlane, kPanarity);

		//- Test for intersection between the circle's plane and Zaxis of viewplane
		AcGeLine3d line;
		line.set(viewplane.pointOnPlane(), viewplane.normal());

		AcGePoint3d intersectPnt;
		if (entPlane.intersectWith(line, intersectPnt) == Adesk::kTrue)
		{
			AcDbExtents extents;
			if (reg->getGeomExtents(extents) != Acad::eOk)
				return (Acad::eInvalidInput);

			const double diagonal = extents.minPoint().distanceTo(extents.maxPoint());
			objWidth = diagonal;
			objHeight = diagonal;

			//- now calculate closest edge to the intersection point
			//- and set the origin and X axis accordingly

			AcGePoint3d origin;
			AcGeVector3d xAxis, yAxis;
			entPlane.getCoordSystem(origin, xAxis, yAxis);
			distToEdge = origin.distanceTo(intersectPnt);

			AcGePlane newPlane(entPlane);
			results.append(newPlane);
			return Acad::eOk;

		}
		return Acad::eNotApplicable;
	}
	return Acad::eNotApplicable;
}
	

	
	
 
	

// My Command
void MyBox()
{	
	double dHeight = 0;
	double dWidth = 0;
	double dLength = 0;	
	if (getPrompt(_T("\nWidth <10>:"),dWidth) == RTNORM)
	{
		if (getPrompt(_T("\nLength <10>:"),dLength) == RTNORM)
		{
			if (getPrompt(_T("\nHeight <10>:"),dHeight,true) == RTNORM)
			{
				CMyBox *myBox = NULL;
				myBox = new CMyBox(dWidth,dLength,dHeight);
			
				AcDbObjectId idOut;
				if (dHeight>CALC_EPSILON)
				{
					// create Box
					myBox->createSolid();
				}
				else
				{
					// draw rectang in subWorldDraw
				//	myBox->createPline();
					
				}
			
				postToDatabase(NULL,myBox,idOut);
			}
		}
	}
}



// ask user to input value

static AsdkUCSPE *pUCSPE = NULL;
//**************************************************************
extern "C"
AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void *pkt)
//**************************************************************
{
	switch (msg)
		{
		case AcRx::kInitAppMsg:
			acrxDynamicLinker->unlockApplication(pkt);
			acrxDynamicLinker->registerAppMDIAware(pkt);
			/*Load commands*/
			/*Instantiate*/
			CMyBox::rxInit();
			/*Need this to add the AcRxClass objects created by rxInit() into the class tree*/
			acrxBuildClassHierarchy();/*use this for derived classes*/
			{
				pUCSPE = new AsdkUCSPE();
				AcRxObject* pPex = CMyBox::desc()->addX(AcDbDynamicUCSPE::desc(), pUCSPE);
				
			}
			
			acedRegCmds->addCommand(_T("TestCmd"), _T("MyBox"), _T("MyBox"), ACRX_CMD_MODAL, MyBox);
		  
			break;
		case AcRx::kUnloadAppMsg:
			if (pUCSPE) 
				{
				CMyBox::desc()->delX(AcDbDynamicUCSPE::desc());
				delete pUCSPE;
				pUCSPE = NULL;
				}
			
			/*unload commands*/
			deleteAcRxClass(CMyBox::desc());
			acedRegCmds->removeGroup(_T("TestCmd"));
			break;
		default:
			break;
	}
	return AcRx::kRetOK;
}