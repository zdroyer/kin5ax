// CToolHolder: the solid that hangs from the tool mount, which is the terminal of the tool branch.
// Up to three cylinders, measured from the mount downwards, and the designation of the holder, which
// the library carries without reading. The catalogue is the holders a five-axis machine usually
// carries, so that one can be picked from a list instead of measured.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "ToolHolder.h"

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

//---- the holder of one cylinder, which is what a holder was before it had sections

TEST(ToolHolder, OneCylinderIsOneSection)
{
	CToolHolder oHolder(0.05,0.03);
	ASSERT_EQ(size_t(1),oHolder.GetSections().size());
	EXPECT_DOUBLE_EQ(0.05,oHolder.GetLength());
	EXPECT_DOUBLE_EQ(0.03,oHolder.GetRadius());
	EXPECT_TRUE(oHolder.HasBody());
	EXPECT_FALSE(oHolder.IsEmpty());
}

TEST(ToolHolder, NothingGivenIsNoHolderAtAll)
{
	CToolHolder oHolder;
	EXPECT_TRUE(oHolder.GetSections().empty());
	EXPECT_TRUE(oHolder.IsEmpty());
	EXPECT_FALSE(oHolder.HasBody());
	EXPECT_DOUBLE_EQ(0.0,oHolder.GetLength());
	EXPECT_DOUBLE_EQ(0.0,oHolder.GetRadius());
}

TEST(ToolHolder, ALengthWithoutARadiusIsADistanceOnly)
{
	//the tool hangs that far below the mount, and there is no solid to meet anything
	CToolHolder oHolder(0.05,0.0);
	EXPECT_DOUBLE_EQ(0.05,oHolder.GetLength());
	EXPECT_FALSE(oHolder.HasBody());
	EXPECT_FALSE(oHolder.IsEmpty());
}

//---- several cylinders

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

TEST(ToolHolder, AModelAloneIsAName)
{
	//what the machine usually carries, for a host to resolve against the holders it has of its own
	CToolHolder oHolder("HSK63A ER32",std::vector<CToolHolderSection>());
	EXPECT_FALSE(oHolder.IsEmpty());
	EXPECT_FALSE(oHolder.HasBody());
	EXPECT_DOUBLE_EQ(0.0,oHolder.GetLength());
}

//---- the catalogue

TEST(ToolHolderCatalogue, HoldsBetweenFiveAndTenUsualHolders)
{
	ASSERT_GE(ToolHolderCatalogueSize(),size_t(5));
	ASSERT_LE(ToolHolderCatalogueSize(),size_t(10));
}

TEST(ToolHolderCatalogue, EveryEntryIsANamedSolidOfPlausibleSize)
{
	for(size_t i=0;i<ToolHolderCatalogueSize();i++){
		CToolHolder oHolder = ToolHolderFromCatalogue(i);
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
	const std::string sModel = ToolHolderFromCatalogue(0).GetModel();
	EXPECT_TRUE(ToolHolderByModel(sModel)==ToolHolderFromCatalogue(0));
	//a host that wants to be forgiving about spelling can be; guessing here would put a holder of
	//the wrong length on the machine
	EXPECT_TRUE(ToolHolderByModel("bt40 er32").IsEmpty());
	EXPECT_TRUE(ToolHolderByModel("no such holder").IsEmpty());
	EXPECT_TRUE(ToolHolderByModel("").IsEmpty());
}

TEST(ToolHolderCatalogue, IsEmptyPastItsEnd)
{
	EXPECT_TRUE(ToolHolderFromCatalogue(ToolHolderCatalogueSize()).IsEmpty());
	EXPECT_FALSE(ToolHolderFromCatalogue(ToolHolderCatalogueSize()-1).IsEmpty());
}
