// CChain::GetActualFrames: one frame per segment, in the frame of the machine base. The same walk the
// solver and the collision test do inside themselves, so the frames have to agree with both.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "ArgsMyException.h"
#include "double_tools.h"
#include <random>

using namespace kin5ax;

namespace {

//every element of both matrices, so that a difference in the rotation is not hidden by the translation
void ExpectSameTransformation(const CTransformation &a, const CTransformation &b, const std::string &sWhat
	,double dTolerance = 1E-12)
{
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			EXPECT_NEAR(a.GetAt(i,j),b.GetAt(i,j),dTolerance)<<sWhat<<" at "<<i<<","<<j;
}

} // namespace

class SegmentFrameChain : public ::testing::TestWithParam<std::string> {};

struct SegmentFrameChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//the last output frame of each part is what the chain itself solves with
TEST_P(SegmentFrameChain, TheLastFrameOfAPartIsItsTerminal)
{
	CChain oChain = LoadChain(GetParam());
	std::mt19937 oGenerator(23);
	for(int s=0;s<20;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		std::vector<CSegmentFrame> vTool = oChain.GetActualFrames(CHAIN_PART_SPINDLE);
		std::vector<CSegmentFrame> vMaterial = oChain.GetActualFrames(CHAIN_PART_TABLE);
		ASSERT_EQ(oChain.m_vToolPart.size(),vTool.size());
		ASSERT_EQ(oChain.m_vMaterialHolderPart.size(),vMaterial.size());

		//the nose of the spindle, plus the hanging tool, is the transformation to the programmed point
		ExpectSameTransformation(oChain.GetActualToToolTransformation()
			,vTool.back().m_trOutput * oChain.GetSpindleNoseToToolPointTransformation()
			,"the tool point of sample "+std::to_string(s));
		//and the inverse walk of the material holder part undoes the last frame of it
		ExpectSameTransformation(CTransformation(1.0)
			,oChain.GetActualToMaterialInvertedTransformation() * vMaterial.back().m_trOutput
			,"the material frame of sample "+std::to_string(s));
	}
}

//the joint frame and the output frame differ by the motion of the joint, and the output frame of one
//segment is the input frame of the next
TEST_P(SegmentFrameChain, TheFramesChainTogether)
{
	CChain oChain = LoadChain(GetParam());
	std::mt19937 oGenerator(29);
	const CHAIN_PART vParts[2] = {CHAIN_PART_SPINDLE,CHAIN_PART_TABLE};
	for(int s=0;s<10;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		for(int p=0;p<2;p++){
			const std::vector<CChainSegment> &vPart = oChain.GetPart(vParts[p]);
			std::vector<CSegmentFrame> vFrames = oChain.GetActualFrames(vParts[p]);
			CTransformation trInput(1.0);		//the machine base
			for(size_t i=0;i<vPart.size();i++){
				const double q = vPart[i].m_dActualParameterValue;
				ExpectSameTransformation(trInput * vPart[i].GetConstantTransformation()
					,vFrames[i].m_trJoint,"the joint frame of segment "+std::to_string(i));
				ExpectSameTransformation(vFrames[i].m_trJoint * vPart[i].GetVariablePartTransformation(q)
					,vFrames[i].m_trOutput,"the output frame of segment "+std::to_string(i));
				//and both together are the one transformation the solver uses
				ExpectSameTransformation(trInput * vPart[i].GetParametrizedTransformation(q)
					,vFrames[i].m_trOutput,"the parametrized transformation of segment "+std::to_string(i));
				trInput = vFrames[i].m_trOutput;
			}
		}
	}
}

//the Z of the joint frame is the axis of the joint: a revolute link leaves the points of that axis where
//they are, a prismatic link moves everything along it
TEST_P(SegmentFrameChain, TheZOfTheJointFrameIsTheAxisOfTheJoint)
{
	CChain oChain = LoadChain(GetParam());
	oChain.ZeroAllParameters();
	const CHAIN_PART vParts[2] = {CHAIN_PART_SPINDLE,CHAIN_PART_TABLE};
	for(int p=0;p<2;p++){
		std::vector<CChainSegment> &vPart = oChain.GetPart(vParts[p]);
		for(size_t i=0;i<vPart.size();i++){
			if(!vPart[i].IsRegularJoint())
				continue;
			const bool bRevolute = vPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK;
			const CSegmentFrame oZero = oChain.GetActualFrame(vParts[p],i);
			//a point one unit up the local Z of the joint, and the same point after moving the joint
			const CR3Point pOnAxis = oZero.m_trOutput * CR3Point(0.0,0.0,1.0);
			vPart[i].m_dActualParameterValue = bRevolute ? 0.7 : 0.123;
			const CSegmentFrame oMoved = oChain.GetActualFrame(vParts[p],i);
			const CR3Point pMoved = oMoved.m_trOutput * CR3Point(0.0,0.0,1.0);
			ExpectSameTransformation(oZero.m_trJoint,oMoved.m_trJoint
				,"the joint frame does not depend on the joint value");
			if(bRevolute){
				EXPECT_NEAR(0.0,d_E(pOnAxis,pMoved),1E-12)<<"a point of the axis stays where it is";
			} else {
				//the point moved by the joint value, along the Z of the joint frame
				CR3Vector r3vAxis = oZero.m_trJoint * CR3Vector(0.0,0.0,1.0);
				CR3Point pExpected = pOnAxis + (r3vAxis.GetUnitVector()*0.123);
				EXPECT_NEAR(0.0,d_E(pExpected,pMoved),1E-12)<<"the point slid along the axis of the joint";
			}
			vPart[i].m_dActualParameterValue = 0.0;
		}
	}
}

INSTANTIATE_TEST_CASE_P(AllChains, SegmentFrameChain, ::testing::ValuesIn(GetChainFiles()), SegmentFrameChainName());

TEST(SegmentFrame, OneSegmentAgreesWithTheWholePart)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	std::mt19937 oGenerator(31);
	oChain.SetParameters(RandomJointValues(oChain,oGenerator));
	const CHAIN_PART vParts[2] = {CHAIN_PART_SPINDLE,CHAIN_PART_TABLE};
	for(int p=0;p<2;p++){
		std::vector<CSegmentFrame> vFrames = oChain.GetActualFrames(vParts[p]);
		ASSERT_GT(vFrames.size(),(size_t)0);
		for(size_t i=0;i<vFrames.size();i++){
			CSegmentFrame oOne = oChain.GetActualFrame(vParts[p],i);
			ExpectSameTransformation(vFrames[i].m_trJoint,oOne.m_trJoint,"joint frame "+std::to_string(i));
			ExpectSameTransformation(vFrames[i].m_trOutput,oOne.m_trOutput,"output frame "+std::to_string(i));
		}
		EXPECT_THROW(oChain.GetActualFrame(vParts[p],vFrames.size()),CArgsMyException)<<"past the end";
	}
}

//a chain built in code, so the numbers are known by hand: a prismatic link 0.3 up the Z of the base,
//then the tool terminal 0.2 further up
TEST(SegmentFrame, TheFramesOfAChainBuiltByHand)
{
	CChain oChain;
	oChain.m_vToolPart.clear();
	oChain.m_vMaterialHolderPart.clear();
	CR3Vector r3vX(1.0,0.0,0.0), r3vNone(0.0,0.0,0.0);
	oChain.m_vToolPart.push_back(CChainSegment(CHAIN_SEGMENT_PRISMATIC_LINK,r3vX,0.0,CR3Vector(0.0,0.0,0.3)));
	oChain.m_vToolPart.push_back(CChainSegment(CHAIN_SEGMENT_TOOL_SPINDLE_NOSE,r3vX,0.0,CR3Vector(0.0,0.0,0.2)));
	oChain.m_vMaterialHolderPart.push_back(CChainSegment(CHAIN_SEGMENT_TABLE,r3vX,0.0,r3vNone));
	oChain.m_vToolPart[0].m_dActualParameterValue = 0.05;

	std::vector<CSegmentFrame> v = oChain.GetActualFrames(CHAIN_PART_SPINDLE);
	ASSERT_EQ((size_t)2,v.size());
	CR3Point pJoint = v[0].m_trJoint * CR3Point(0.0,0.0,0.0);
	EXPECT_NEAR(0.3,pJoint.GetZ(),1E-12)<<"the boxes of the link sit where its constant translation put it";
	CR3Point pOut = v[0].m_trOutput * CR3Point(0.0,0.0,0.0);
	EXPECT_NEAR(0.35,pOut.GetZ(),1E-12)<<"and the joint slid 0.05 further";
	CR3Point pNose = v[1].m_trOutput * CR3Point(0.0,0.0,0.0);
	EXPECT_NEAR(0.55,pNose.GetZ(),1E-12)<<"the nose of the spindle, 0.2 above the link";
	EXPECT_NEAR(0.0,pNose.GetX(),1E-12);
	EXPECT_NEAR(0.0,pNose.GetY(),1E-12);
}
