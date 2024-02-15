#include "APIEnvir.h"
#include "ACAPinc.h"

#include "ResourceIds.hpp"
#include "DGModule.hpp"

#include "StringConversion.hpp"

static const GSResID AddOnInfoID			= ID_ADDON_INFO;
	static const Int32 AddOnNameID			= 1;
	static const Int32 AddOnDescriptionID	= 2;

static const short AddOnMenuID				= ID_ADDON_MENU;
	static const Int32 AddOnCommandID		= 1;

class ExampleDialog :	public DG::ModalDialog,
						public DG::PanelObserver,
						public DG::ButtonItemObserver,
						public DG::CompoundItemObserver
{
public:
	enum DialogResourceIds
	{
		ExampleDialogResourceId = ID_ADDON_DLG,
		OKButtonId = 1,
		CancelButtonId = 2,
		SeparatorId = 3
	};

	ExampleDialog () :
		DG::ModalDialog (ACAPI_GetOwnResModule (), ExampleDialogResourceId, ACAPI_GetOwnResModule ()),
		okButton (GetReference (), OKButtonId),
		cancelButton (GetReference (), CancelButtonId),
		separator (GetReference (), SeparatorId)
	{
		AttachToAllItems (*this);
		Attach (*this);
	}

	~ExampleDialog ()
	{
		Detach (*this);
		DetachFromAllItems (*this);
	}

private:
	virtual void PanelResized (const DG::PanelResizeEvent& ev) override
	{
		BeginMoveResizeItems ();
		okButton.Move (ev.GetHorizontalChange (), ev.GetVerticalChange ());
		cancelButton.Move (ev.GetHorizontalChange (), ev.GetVerticalChange ());
		separator.MoveAndResize (0, ev.GetVerticalChange (), ev.GetHorizontalChange (), 0);
		EndMoveResizeItems ();
	}

	virtual void ButtonClicked (const DG::ButtonClickEvent& ev) override
	{
		if (ev.GetSource () == &okButton) {
			PostCloseRequest (DG::ModalDialog::Accept);
		} else if (ev.GetSource () == &cancelButton) {
			PostCloseRequest (DG::ModalDialog::Cancel);
		}
	}

	DG::Button		okButton;
	DG::Button		cancelButton;
	DG::Separator	separator;
};

//https://archicadapi.graphisoft.com/archicad-maze-generator-add-on-tutorial-part-1
static void CountNumberOfWalls()
{
	GSErrCode            err;
	API_SelectionInfo    selectionInfo;
	//API_Elem_Head        tElemHead;
	GS::Array<API_Neig>  selNeigs;

	err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
	BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);

	/*if (err == APIERR_NOSEL)
		err = NoError;*/

	//API_MeshID 

	GS::Array<API_Guid> elemList;
	ACAPI_Element_GetElemList(API_MeshID, &elemList, APIFilt_OnActFloor);

	if (elemList.GetSize() > 0)
	{
		API_Element mesh = {};
		API_ElementMemo  memo;
		mesh.header.guid = elemList[0];
		GSErrCode err = ACAPI_Element_Get(&mesh);

		if (err != NoError)
			return;

		//https://archicadapi.graphisoft.com/documentation/api_gridmesh
		if (mesh.header.hasMemo) {
			err = ACAPI_Element_GetMemo(mesh.header.guid, &memo);
			if (err != NoError) {
				return;
			}
			
			//here deeper

			ACAPI_WriteReport("i: (x, y, z)", false);
			ACAPI_WriteReport("------------", false);
			for (GS::Int32 i = 1; i <= mesh.mesh.poly.nCoords; ++i) {
				ACAPI_WriteReport("%d: (%f, %f, %f): ", false, i, (*memo.coords)[i].x, (*memo.coords)[i].y, (*memo.meshPolyZ)[i]);
			}

			ACAPI_DisposeElemMemoHdls(&memo);

			//API_ElemInfo3D mesh3D = {};

			//err = ACAPI_Element_Get3DInfo(mesh.header, &mesh3D);

			//if (err != NoError)
			//	return;

			//API_Component3D          comp3D;
			//BNZeroMemory(&comp3D, sizeof(API_Component3D));
			//comp3D.header.index = 0;
			//comp3D.header.typeID = API_BodyID;
			/*err = ACAPI_ModelAccess_GetComponent(&comp3D);

			if (err != NoError)
				return;*/
		}
	}	

	//boxes https://archicadapi.graphisoft.com/documentation/api_eleminfo3d

	GS::Array<API_Guid> inds = GS::Array<API_Guid>();

	if (selectionInfo.typeID != API_SelEmpty) 
	{		
		for (const API_Neig& selNeig : selNeigs) {
			/*tElemHead.typeID = APIAny_​NeigIDToElemTypeID(selNeig.neigID);
			if (tElemHead.typeID != API_DimensionID)
				continue;*/

			if (!ACAPI_Element_Filter(selNeig.guid, APIFilt_IsEditable))
				continue;

			/*tElemHead.guid = selNeig.guid;
			if (ACAPI_Element_GetHeader(&tElemHead) != NoError)
				continue;*/

			// Add dimension to the array
			inds.Push(selNeig.guid);

			API_Element element = {};
			element.header.guid = selNeig.guid;
			GSErrCode err = ACAPI_Element_Get(&element);
			if (err != NoError)
				continue;

			
		}
	}

	DG::InformationAlert(GS::ValueToUniString(inds.GetSize()) + " objects selected, pokakakat'", "", "OK");
}

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case AddOnCommandID:
					{
						CountNumberOfWalls();
					}
					break;
			}
			break;
	}
	return NoError;
}

API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}

GSErrCode RegisterInterface (void)
{
#ifdef ServerMainVers_2700
	return ACAPI_MenuItem_RegisterMenu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
#else
	return ACAPI_Register_Menu (AddOnMenuID, 0, MenuCode_Tools, MenuFlag_Default);
#endif
}

GSErrCode Initialize (void)
{
#ifdef ServerMainVers_2700
	return ACAPI_MenuItem_InstallMenuHandler (AddOnMenuID, MenuCommandHandler);
#else
	return ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
#endif
}

GSErrCode FreeData (void)
{
	return NoError;
}
