// CWorkpieceHolder: what is bolted to the table and where it holds the part. A frame given as a
// translation and a rotation, and boxes for the solids. The catalogue begins with the two that are
// not holders at all: the part on the table itself, and the one a host fills in.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "WorkpieceHolder.h"
#include "ToolHolder.h"

using namespace kin5ax;

TEST(WorkpieceHolder, NothingGivenIsThePartOnTheTable)
{
	const CWorkpieceHolder oHolder;
	EXPECT_TRUE(oHolder.IsEmpty());
	EXPECT_TRUE(oHolder.IsFrameIdentity());
	EXPECT_FALSE(oHolder.HasBody());
	const CR3Point p(0.1,0.2,0.3);
	const CR3Point pMoved = oHolder.GetTransformation()*p;
	EXPECT_NEAR(0.0,d_E(p,pMoved),1E-12)<<"the identity moves nothing";
}

TEST(WorkpieceHolder, TheFrameIsATranslationAfterARotation)
{
	//the part sits 80 mm up, its frame turned a quarter turn about OZ: the X of the part runs along
	//the Y of the table, as a segment's constant transformation composes
	const CWorkpieceHolder oHolder("X_VICE",CR3Vector(0.0,0.0,0.08),CR3Vector(0.0,0.0,1.0),90.0);
	EXPECT_FALSE(oHolder.IsFrameIdentity());
	const CR3Point pOnTable = oHolder.GetTransformation()*CR3Point(0.05,0.0,0.0);
	EXPECT_NEAR(0.00,pOnTable.GetX(),1E-12);
	EXPECT_NEAR(0.05,pOnTable.GetY(),1E-12);
	EXPECT_NEAR(0.08,pOnTable.GetZ(),1E-12);
	//and back again
	const CR3Point pBack = oHolder.GetInvertedTransformation()*pOnTable;
	EXPECT_NEAR(0.0,d_E(CR3Point(0.05,0.0,0.0),pBack),1E-12);
}

TEST(WorkpieceHolder, TwoHoldersAreEqualWhenTheNameTheFrameAndTheSolidsAre)
{
	const CWorkpieceHolder oA("X_VICE",CR3Vector(0.0,0.0,0.08),CR3Vector(0.0,0.0,1.0),0.0);
	CWorkpieceHolder oB("X_VICE",CR3Vector(0.0,0.0,0.08),CR3Vector(0.0,0.0,1.0),0.0);
	EXPECT_TRUE(oA==oB);
	oB.SetBoxes(std::vector<CR3Box>(1,CR3Box(-0.1,0.1,-0.1,0.1,0.0,0.08)));
	EXPECT_TRUE(oA!=oB)<<"the solids are part of what the file said";
}

//---- the catalogue

TEST(WorkpieceHolderCatalogue, BeginsWithTheTwoThatAreNotHolders)
{
	ASSERT_GE(WorkpieceHolderCatalogueSize(),size_t(8));
	const CWorkpieceHolder oNone = WorkpieceHolderFromCatalogue(0);
	EXPECT_EQ(std::string(szNO_HOLDER),oNone.GetModel());
	EXPECT_TRUE(oNone.IsFrameIdentity())<<"the part is bolted to the table itself";
	EXPECT_FALSE(oNone.HasBody());

	const CWorkpieceHolder oCustom = WorkpieceHolderFromCatalogue(1);
	EXPECT_EQ(std::string(szCUSTOM_HOLDER),oCustom.GetModel());
	EXPECT_TRUE(oCustom.IsFrameIdentity())<<"a host fills in the frame and the solids";
	EXPECT_FALSE(oCustom.HasBody());
}

TEST(WorkpieceHolderCatalogue, EveryRealHolderHoldsThePartOffTheTableAndHasASolid)
{
	for(size_t i=2;i<WorkpieceHolderCatalogueSize();i++){
		const CWorkpieceHolder oHolder = WorkpieceHolderFromCatalogue(i);
		SCOPED_TRACE(oHolder.GetModel());
		EXPECT_FALSE(oHolder.GetModel().empty());
		EXPECT_FALSE(oHolder.IsFrameIdentity())<<"it holds the part somewhere else than the table";
		EXPECT_TRUE(oHolder.HasBody())<<"and it is a solid the tool can run into";
		//metres: a holder stands on a table and fits a workshop
		const CR3Box oBox = oHolder.GetBoxes()[0];
		EXPECT_GT(oBox.GetMaxPoint().GetZ(),0.01);
		EXPECT_LT(oBox.GetMaxPoint().GetZ(),1.0);
	}
}

TEST(WorkpieceHolderCatalogue, HasTheVicesOfBothDirectionsAndTheyAreTwoJawsAndAGap)
{
	CWorkpieceHolder oX, oY;
	ASSERT_TRUE(WorkpieceHolderByModel("X_VICE",oX));
	ASSERT_TRUE(WorkpieceHolderByModel("Y_VICE",oY));
	ASSERT_EQ((size_t)2,oX.GetBoxes().size())<<"a vice is two jaws, and a tool can come between them";
	ASSERT_EQ((size_t)2,oY.GetBoxes().size());
	EXPECT_DOUBLE_EQ(oX.GetTranslation().GetZ(),oY.GetTranslation().GetZ())
		<<"the same vice holds the part at the same height either way round";
	EXPECT_DOUBLE_EQ(0.0,oX.GetRotationAngleDegrees())<<"the jaws say which way it opens, not a turn";
	EXPECT_DOUBLE_EQ(0.0,oY.GetRotationAngleDegrees());

	//what the two numbers of a vice are, read back off its jaws
	R3_AXIS nAxis = R3_AXIS_Z;
	double dOpening = 0.0, dJawLength = 0.0;
	ASSERT_TRUE(ViceParameters(oX,nAxis,dOpening,dJawLength));
	EXPECT_EQ(R3_AXIS_X,nAxis);
	EXPECT_DOUBLE_EQ(0.100,dOpening)<<"a 125 mm vice opened 100 mm";
	EXPECT_DOUBLE_EQ(0.125,dJawLength);
	ASSERT_TRUE(ViceParameters(oY,nAxis,dOpening,dJawLength));
	EXPECT_EQ(R3_AXIS_Y,nAxis);
	EXPECT_DOUBLE_EQ(0.100,dOpening);

	//the part sits on the jaw tops, in the middle of the opening
	EXPECT_DOUBLE_EQ(dVICE_JAW_HEIGHT,oX.GetTranslation().GetZ());
	EXPECT_DOUBLE_EQ(dVICE_JAW_HEIGHT,oX.GetBoxes()[0].GetMaxPoint().GetZ());
	EXPECT_DOUBLE_EQ(0.0,oX.GetBoxes()[0].GetMinPoint().GetZ())<<"and stands on the table";
	EXPECT_DOUBLE_EQ(-0.05,oX.GetBoxes()[0].GetMaxPoint().GetX())<<"the near face of the first jaw";
	EXPECT_DOUBLE_EQ(0.05,oX.GetBoxes()[1].GetMinPoint().GetX());
}

TEST(WorkpieceHolderCatalogue, AViceIsBuiltFromItsOpeningAndItsJawLength)
{
	const CWorkpieceHolder oWide = MakeVice("X_VICE",R3_AXIS_X,0.200,0.160);
	R3_AXIS nAxis = R3_AXIS_Z;
	double dOpening = 0.0, dJawLength = 0.0;
	ASSERT_TRUE(ViceParameters(oWide,nAxis,dOpening,dJawLength));
	EXPECT_EQ(R3_AXIS_X,nAxis);
	EXPECT_DOUBLE_EQ(0.200,dOpening)<<"what was asked for comes back";
	EXPECT_DOUBLE_EQ(0.160,dJawLength);
	//a jaw is as thick and as tall as the catalogue says, which nobody is asked for
	const CR3Point pMin = oWide.GetBoxes()[0].GetMinPoint();
	const CR3Point pMax = oWide.GetBoxes()[0].GetMaxPoint();
	EXPECT_DOUBLE_EQ(dVICE_JAW_THICKNESS,pMax.GetX()-pMin.GetX());
	EXPECT_DOUBLE_EQ(dVICE_JAW_HEIGHT,pMax.GetZ()-pMin.GetZ());

	//and anything that is not two jaws is not a vice
	CWorkpieceHolder oPlate;
	ASSERT_TRUE(WorkpieceHolderByModel("FIXTURE_PLATE",oPlate));
	EXPECT_FALSE(ViceParameters(oPlate,nAxis,dOpening,dJawLength));
	EXPECT_FALSE(ViceParameters(CWorkpieceHolder(),nAxis,dOpening,dJawLength));
}

TEST(WorkpieceHolderCatalogue, ATombstoneHoldsThePartOnItsFace)
{
	//the one holder whose frame is turned: the Z of the part points out along the X of the table
	CWorkpieceHolder oHolder;
	ASSERT_TRUE(WorkpieceHolderByModel("TOMBSTONE",oHolder));
	const CR3Vector r3vUp = oHolder.GetTransformation()*CR3Vector(0.0,0.0,1.0);
	EXPECT_NEAR(1.0,r3vUp.GetX(),1E-9)<<"the part faces along the X of the table";
	EXPECT_NEAR(0.0,r3vUp.GetZ(),1E-9);
	EXPECT_GT(oHolder.GetTranslation().GetZ(),0.1)<<"and it is held up the column";
}

TEST(WorkpieceHolderCatalogue, IsSearchedByTheDesignationExactly)
{
	CWorkpieceHolder oHolder;
	EXPECT_TRUE(WorkpieceHolderByModel(szNO_HOLDER,oHolder));
	EXPECT_TRUE(WorkpieceHolderByModel(szCUSTOM_HOLDER,oHolder));
	EXPECT_FALSE(WorkpieceHolderByModel("x_vice",oHolder));
	EXPECT_FALSE(WorkpieceHolderByModel("",oHolder));
	EXPECT_TRUE(WorkpieceHolderFromCatalogue(WorkpieceHolderCatalogueSize()).IsEmpty());
}

TEST(WorkpieceHolderCatalogue, NamesNoHolderTwice)
{
	for(size_t i=0;i<WorkpieceHolderCatalogueSize();i++)
		for(size_t j=i+1;j<WorkpieceHolderCatalogueSize();j++)
			EXPECT_NE(WorkpieceHolderFromCatalogue(i).GetModel()
				,WorkpieceHolderFromCatalogue(j).GetModel());
}
