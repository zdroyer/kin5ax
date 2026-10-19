#pragma once

#include "Chain.h"

/**
 * @brief Reads a kinematic chain from a file: the JSON form (.kc.json) and nothing else.
 *
 * The XML form (.kc) was retired. It is neither read nor written by this library any more, and a
 * file still in it is refused by name with a word about tools/kc2json, which converts it. This is
 * what an application opens a chain with.
 */
namespace kin5ax {

class CChainFileReader
{
public:
	/// True when the path names a file in the retired XML form, by its extension or its first character.
	static bool IsRetiredXml(const std::string &sFilePath);
	/// @throws CGeneratorException with the file name and the place of every problem.
	static CChain ReadFile(const std::string &sFilePath);
};

} // namespace kin5ax
