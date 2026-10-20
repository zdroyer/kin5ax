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

TEST(WorkpieceHolderCatalogue, HasTheVicesOfBothDirectionsAndTheyDifferByTheirTurn)
{
	CWorkpieceHolder oX, oY;
	ASSERT_TRUE(WorkpieceHolderByModel("X_VICE",oX));
	ASSERT_TRUE(WorkpieceHolderByModel("Y_VICE",oY));
	EXPECT_DOUBLE_EQ(oX.GetTranslation().GetZ(),oY.GetTranslation().GetZ())
		<<"the same vice holds the part at the same height either way round";
	EXPECT_NE(oX.GetRotationAngleDegrees(),oY.GetRotationAngleDegrees())
		<<"what tells them apart is which way the jaws open";
	//the body follows the turn: the X vice is long across X, the Y vice long across Y
	EXPECT_GT(oX.GetBoxes()[0].GetMaxPoint().GetX(),oX.GetBoxes()[0].GetMaxPoint().GetY());
	EXPECT_GT(oY.GetBoxes()[0].GetMaxPoint().GetY(),oY.GetBoxes()[0].GetMaxPoint().GetX());
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
