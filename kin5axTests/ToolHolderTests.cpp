// CToolHolder: what is clamped in the nose of the tool spindle, and what the tool sticks out of.
// Up to three cylinders measured from the nose downwards, and the designation of the holder, which
// the library carries without reading. The catalogue is the holders a five-axis machine usually
// carries, so that one can be picked from a list instead of measured, and NO_HOLDER is the first of
// them: a tool clamped straight into the spindle.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "ToolHolder.h"
#include "MillingToolBall.h"

using namespace kin5ax;

//---- one cylinder

TEST(ToolHolderSection, IsAsGivenAndNeverNegative)
{
	CToolHolderSection oSection(0.05,0.03);
	EXPECT_DOUBLE_EQ(0.05,oSection.GetLength());
	EXPECT_DOUBLE_EQ(0.03,oSection.GetRadius());
	//a length is a distance; which way round somebody wrote it is not a shape
	CToolHolderSection oNegative(-0.05,-0.03);
	EXPECT_DOUBLE_EQ(0.05,oNegative.GetLength());
	EXPECT_DOUBLE_EQ(0.03,oNegative.GetRadius());
	EXPECT_TRUE(oSection==oNegative);
}

//---- the holder

TEST(ToolHolder, OneCylinderIsOneSection)
{
	CToolHolder oHolder(0.05,0.03);
	ASSERT_EQ(size_t(1),oHolder.GetSections().size());
	EXPECT_DOUBLE_EQ(0.05,oHolder.GetLength());
	EXPECT_DOUBLE_EQ(0.03,oHolder.GetRadius());
	EXPECT_TRUE(oHolder.HasBody());
	EXPECT_FALSE(oHolder.IsNoHolder());
}

TEST(ToolHolder, NothingGivenIsNoHolderAtAll)
{
	CToolHolder oHolder;
	EXPECT_TRUE(oHolder.GetSections().empty());
	EXPECT_TRUE(oHolder.IsEmpty());
	EXPECT_TRUE(oHolder.IsNoHolder());
	EXPECT_FALSE(oHolder.HasBody());
	EXPECT_DOUBLE_EQ(0.0,oHolder.GetLength());
}

TEST(ToolHolder, TheLengthIsTheSumAndTheRadiusTheWidest)
{
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(0.065,0.0445));
	v.push_back(CToolHolderSection(0.025,0.027));
	v.push_back(CToolHolderSection(0.035,0.025));
	CToolHolder oHolder("BT40 ER32",v);
	EXPECT_EQ(std::string("BT40 ER32"),oHolder.GetModel());
	ASSERT_EQ(size_t(3),oHolder.GetSections().size());
	EXPECT_DOUBLE_EQ(0.125,oHolder.GetLength());
	EXPECT_DOUBLE_EQ(0.0445,oHolder.GetRadius())<<"a bounding test wants the widest, not the first";
}

TEST(ToolHolder, ASectionBeginsWhereTheOnesAboveItEnd)
{
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(0.06,0.04));
	v.push_back(CToolHolderSection(0.02,0.03));
	v.push_back(CToolHolderSection(0.04,0.02));
	CToolHolder oHolder("",v);
	EXPECT_DOUBLE_EQ(0.00,oHolder.GetSectionTop(0))<<"the first is against the spindle";
	EXPECT_DOUBLE_EQ(0.06,oHolder.GetSectionTop(1));
	EXPECT_DOUBLE_EQ(0.08,oHolder.GetSectionTop(2));
	EXPECT_DOUBLE_EQ(oHolder.GetLength(),oHolder.GetSectionTop(3))<<"past the last is the end of it";
}

TEST(ToolHolder, MoreSectionsThanAHolderHasAreDropped)
{
	std::vector<CToolHolderSection> v;
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX+2;i++)
		v.push_back(CToolHolderSection(0.01,0.02));
	CToolHolder oHolder("",v);
	EXPECT_EQ(nTOOL_HOLDER_SECTIONS_MAX,oHolder.GetSections().size());
}

TEST(ToolHolder, ASectionOfNoLengthIsNotAShape)
{
	//two holders that are the same solid must compare equal, so a section of nothing is not kept
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(0.05,0.03));
	v.push_back(CToolHolderSection(0.0,0.02));
	CToolHolder oHolder("",v);
	ASSERT_EQ(size_t(1),oHolder.GetSections().size());
	EXPECT_TRUE(oHolder==CToolHolder(0.05,0.03));
}

TEST(ToolHolder, TwoHoldersAreEqualWhenTheModelAndTheSolidAre)
{
	CToolHolder oHolder(0.05,0.03);
	EXPECT_TRUE(oHolder==CToolHolder(0.05,0.03));
	EXPECT_TRUE(oHolder!=CToolHolder(0.05,0.031));
	CToolHolder oNamed(0.05,0.03);
	oNamed.SetModel("BT40 ER32");
	EXPECT_TRUE(oHolder!=oNamed)<<"the name is part of what the file said";
}

//---- the catalogue

TEST(ToolHolderCatalogue, BeginsWithNoHolderAtAll)
{
	//the tool clamped straight into the spindle, as a holder in the data rather than as a gap in it
	ASSERT_GE(ToolHolderCatalogueSize(),size_t(6));
	const CToolHolder oFirst = ToolHolderFromCatalogue(0);
	EXPECT_EQ(std::string(szNO_HOLDER),oFirst.GetModel());
	EXPECT_TRUE(oFirst.IsNoHolder());
	EXPECT_FALSE(oFirst.HasBody());
	EXPECT_DOUBLE_EQ(0.0,oFirst.GetLength());
	EXPECT_FALSE(oFirst.IsEmpty())<<"it has a name, so it is not nothing";
}

TEST(ToolHolderCatalogue, EveryOtherEntryIsANamedSolidOfPlausibleSize)
{
	for(size_t i=1;i<ToolHolderCatalogueSize();i++){
		const CToolHolder oHolder = ToolHolderFromCatalogue(i);
		SCOPED_TRACE(oHolder.GetModel());
		EXPECT_FALSE(oHolder.GetModel().empty());
		EXPECT_TRUE(oHolder.HasBody());
		EXPECT_LE(size_t(1),oHolder.GetSections().size());
		EXPECT_GE(nTOOL_HOLDER_SECTIONS_MAX,oHolder.GetSections().size());
		//metres, which is the unit of every model shipped with this library: a holder is a hand's
		//length and fits in a spindle
		EXPECT_GT(oHolder.GetLength(),0.02);
		EXPECT_LT(oHolder.GetLength(),0.30);
		EXPECT_GT(oHolder.GetRadius(),0.005);
		EXPECT_LT(oHolder.GetRadius(),0.10);
	}
}

TEST(ToolHolderCatalogue, NamesNoHolderTwice)
{
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		for(size_t j=i+1;j<ToolHolderCatalogueSize();j++)
			EXPECT_NE(ToolHolderFromCatalogue(i).GetModel(),ToolHolderFromCatalogue(j).GetModel());
}

TEST(ToolHolderCatalogue, IsSearchedByTheDesignationExactly)
{
	CToolHolder oHolder;
	EXPECT_TRUE(ToolHolderByModel(ToolHolderFromCatalogue(1).GetModel(),oHolder));
	EXPECT_TRUE(oHolder==ToolHolderFromCatalogue(1));
	EXPECT_TRUE(ToolHolderByModel(szNO_HOLDER,oHolder));
	EXPECT_TRUE(oHolder.IsNoHolder());
	//a host that wants to be forgiving about spelling can be; guessing here would put a holder of
	//the wrong length on the machine
	EXPECT_FALSE(ToolHolderByModel("bt40 er32",oHolder));
	EXPECT_FALSE(ToolHolderByModel("no such holder",oHolder));
	EXPECT_FALSE(ToolHolderByModel("",oHolder));
}

TEST(ToolHolderCatalogue, IsEmptyPastItsEnd)
{
	EXPECT_TRUE(ToolHolderFromCatalogue(ToolHolderCatalogueSize()).IsEmpty());
	EXPECT_FALSE(ToolHolderFromCatalogue(ToolHolderCatalogueSize()-1).IsEmpty());
}

//---- the solids of a tool in such a holder

TEST(ToolHolderOnATool, EveryCylinderOfTheHolderIsPartOfTheEnvelope)
{
	//a 16 mm ball nose, 64 of cutting part and 30 of shank, in a holder of three cylinders: the
	//holder sits on top of what sticks out, so its first cylinder begins 94 above the tip
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(0.050,0.040));
	v.push_back(CToolHolderSection(0.025,0.027));
	v.push_back(CToolHolderSection(0.040,0.025));
	const CMillingToolBall oTool(0.016,0.016,0.064,0.016,0.030,CToolHolder("HSK63A ER32",v));
	EXPECT_TRUE(oTool.HasNonCuttingPart());
	EXPECT_DOUBLE_EQ(0.094,oTool.GetToolLength())<<"the cutting part and the shank stick out";
	EXPECT_DOUBLE_EQ(0.094+0.115,oTool.GetAssemblyLength());

	//the nose of the holder is the narrow one, just above the shank; the taper is the wide one at
	//the top, and a point 30 mm off the axis is inside that and outside the nose
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.030,0.0,0.094+0.100)))<<"inside the taper";
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.030,0.0,0.094+0.020)))<<"outside the nose";
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.020,0.0,0.094+0.020)))<<"inside the nose";
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,0.094+0.116)))<<"above the holder";
}
