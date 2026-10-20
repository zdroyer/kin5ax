// CChain::DetectCollisions with a whole tool assembly: the cutting part, the shank and the holder all meet the
// boxes of the machine, each at its own height below the tool mount.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainJson.h"
#include "MillingToolBall.h"

using namespace kin5ax;

namespace {

const double dRADIUS = 0.016;			//the ball and the shank
const double dCUTTING = 0.064;
const double dSHANK = 0.030;
const double dHOLDER_LENGTH = 0.050;
const double dHOLDER_RADIUS = 0.030;
//below the tool mount: the holder from 0 to -0.05, the shank to -0.08, the cutting part to -0.144
const double dHOLDER_Z = -0.5*dHOLDER_LENGTH;
const double dSHANK_Z = -(dHOLDER_LENGTH+0.5*dSHANK);
const double dCUTTING_Z = -(dHOLDER_LENGTH+dSHANK+0.5*dCUTTING);

//A chain of two straight branches: the tool mount, the workpiece frame and the base are the same frame, and
//the tool hangs below it. The box of the workpiece holder is the only box of the machine.
std::string ChainWithABoxAt(double dZ, double dRadial)
{
	const double d = 0.1*dRADIUS;
	std::string s = R"KC({
  "format": "kin5ax-chain",
  "version": 2,
  "unit": "m",
  "name": "a box under the tool mount",
  "tool_branch": {
    "links": [
      { "type": "linear", "name": "Z", "translation": [0, 0, 0], "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
    ],
    "tool_spindle_nose": { "name": "tool", "translation": [0, 0, 0], "rotation": { "axis": [0, 0, 1], "angle_deg": 0 },
      "tool_holder": { "length": HOLDER_LENGTH, "radius": HOLDER_RADIUS } }
  },
  "workpiece_branch": {
    "links": [
      { "type": "rotary", "name": "C", "translation": [0, 0, 0], "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
    ],
    "table": { "name": "holder", "translation": [0, 0, 0], "rotation": { "axis": [0, 0, 1], "angle_deg": 0 },
      "boxes": [ { "min": [BX1, -D, BZ1], "max": [BX2, D, BZ2] } ] }
  }
}
)KC";
	struct CReplacement { const char *szWhat; double dValue; };
	const CReplacement v[] = {
		{"HOLDER_LENGTH",dHOLDER_LENGTH}, {"HOLDER_RADIUS",dHOLDER_RADIUS},
		{"BX1",dRadial-d}, {"BX2",dRadial+d}, {"BZ1",dZ-d}, {"BZ2",dZ+d}, {"D",d}
	};
	for(const CReplacement &o : v){
		size_t n = s.find(o.szWhat);
		while(n != std::string::npos){
			s.replace(n,strlen(o.szWhat),double_to_string_exact(o.dValue));
			n = s.find(o.szWhat);
		}
	}
	return s;
}

//the tool assembly of the tests: the default tool of the application with a shank and a holder
CMillingToolBall Assembly()
{
	return CMillingToolBall(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
}

//the collision log of the assembly against the box of that chain, empty when nothing collides
std::string CollisionLog(double dZ, double dRadial)
{
	CChain oChain;
	try{
		oChain = CChainJsonReader::Parse(ChainWithABoxAt(dZ,dRadial),"text");
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<"exception: "<<e.Message();
		return std::string();
	}
	EXPECT_TRUE(oChain.HasDefaultToolHolder())<<"the document declares a tool holder";
	CMillingToolBall oTool = Assembly();
	oChain.SetToolOffsetFromTool(oTool);
	std::string sLog;
	return oChain.HasCollision(oTool,sLog) ? sLog : std::string();
}

} // namespace

TEST(ChainToolCollision, EveryPartOfTheAssemblyMeetsABoxOfTheMachine)
{
	EXPECT_FALSE(CollisionLog(dCUTTING_Z,0.0).empty())<<"the cutting part";
	EXPECT_FALSE(CollisionLog(dSHANK_Z,0.0).empty())<<"the shank";
	EXPECT_FALSE(CollisionLog(dHOLDER_Z,0.0).empty())<<"the holder";
	EXPECT_FALSE(CollisionLog(dHOLDER_Z,0.9*dHOLDER_RADIUS).empty())<<"the side of the holder";
}

TEST(ChainToolCollision, TheLogNamesTheHolderAndTheShank)
{
	EXPECT_NE(std::string::npos,CollisionLog(dHOLDER_Z,0.0).find("holder or shank"))<<"the holder is not the cutting part";
	EXPECT_NE(std::string::npos,CollisionLog(dSHANK_Z,0.0).find("holder or shank"));
	EXPECT_EQ(std::string::npos,CollisionLog(dCUTTING_Z,0.0).find("holder or shank"))<<"the cutting part keeps its own message";
}

TEST(ChainToolCollision, ClearOfTheAssemblyNothingCollides)
{
	EXPECT_TRUE(CollisionLog(0.05,0.0).empty())<<"above the tool mount";
	EXPECT_TRUE(CollisionLog(dCUTTING_Z-dCUTTING,0.0).empty())<<"below the tip";
	EXPECT_TRUE(CollisionLog(dSHANK_Z,2.0*dHOLDER_RADIUS).empty())<<"beside the whole assembly";
	EXPECT_TRUE(CollisionLog(dSHANK_Z,1.5*dRADIUS).empty())<<"beside the shank, where only the holder is wider";
}

TEST(ChainToolCollision, WithoutAnAssemblyNothingIsAHolderOrAShank)
{
	//the same chain with a tool that brings neither a shank nor a holder: it hangs only its own cutting length
	//below the mount, so its cutting part occupies the space the holder would have (KnownBugs #68)
	CChain oChain = CChainJsonReader::Parse(ChainWithABoxAt(dHOLDER_Z,0.0),"text");
	CMillingToolBall oBare(dRADIUS,dRADIUS,dCUTTING,dRADIUS);
	oChain.SetToolOffsetFromTool(oBare);
	EXPECT_DOUBLE_EQ(dCUTTING-dRADIUS,oChain.GetToolOffset());
	std::string sLog;
	EXPECT_TRUE(oChain.HasCollision(oBare,sLog))<<"the cutting part reaches the box under the mount";
	EXPECT_EQ(std::string::npos,sLog.find("holder or shank"))<<sLog;
}
