// The FreeCAD machine file bridge of the C interface. Everything the C++ side does is in
// kin5ax/Chain/ChainFcm.*; this file is the boundary, and nothing of the C++ library crosses it.
#include "handles.h"		//brings the prelude of the kin5ax library, which its headers expect first
#include "kin5axfcm.h"
#include "boundary.h"		//the guards and the last message, shared with kin5axc.cpp

#include <string>

using namespace kin5ax;

namespace {

CFcmOptions FromC(const k5_fcm_options *p)
{
	CFcmOptions o;
	if(!p)
		return o;
	o.m_bNegateTableLinearAxes = (p->negate_table_linear_axes!=0);
	o.m_bNegateTableRotaryAxes = (p->negate_table_rotary_axes!=0);
	if(p->machine_name)
		o.m_sMachineName = p->machine_name;
	if(p->template_file_path)
		o.m_sTemplateFilePath = p->template_file_path;
	return o;
}

void CopyRemark(const std::string &sText, char *out, size_t nCapacity)
{
	if(!nCapacity)
		return;
	const size_t n = (sText.size()<nCapacity-1) ? sText.size() : nCapacity-1;
	memcpy(out,sText.c_str(),n);
	out[n] = 0;
}

} // namespace

void k5_fcm_options_default(k5_fcm_options *out)
{
	if(!out)
		return;
	out->negate_table_linear_axes = 0;
	out->negate_table_rotary_axes = 0;
	out->machine_name = NULL;
	out->template_file_path = NULL;
}

k5_status k5_chain_open_fcm(const char *file_path, const k5_fcm_options *options, k5_chain *out)
{
	K5_BEGIN
	K5_NEED(file_path && out,"k5_chain_open_fcm needs a file and somewhere to put the chain")
	*out = NULL;
	k5_chain_t *pChain = new k5_chain_t();
	try{
		pChain->m_oChain = CChainFcmReader::ReadFile(file_path,FromC(options),pChain->m_oFcmReport);
	} catch(...) {
		delete pChain;
		throw;
	}
	*out = pChain;
	return K5_OK;
	K5_END(K5_ERROR_FILE)
}

k5_status k5_chain_write_fcm(k5_chain chain, const k5_fcm_options *options, const char *file_path)
{
	K5_BEGIN
	K5_NEED(chain && file_path,"k5_chain_write_fcm needs a chain and a file")
	//the remarks of this conversion replace whatever the chain carried before it
	CChainFcmWriter::WriteFile(chain->m_oChain,file_path,FromC(options),chain->m_oFcmReport);
	return K5_OK;
	K5_END(K5_ERROR_FILE)
}

k5_status k5_fcm_remark_count(k5_chain chain, size_t *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_fcm_remark_count needs a chain and somewhere to put the count")
	*out = chain->m_oFcmReport.m_vRemarks.size();
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_fcm_remark(k5_chain chain, size_t index, char *out, size_t capacity)
{
	K5_BEGIN
	K5_NEED(chain && out && capacity,"k5_fcm_remark needs a chain and a buffer")
	K5_NEED(index<chain->m_oFcmReport.m_vRemarks.size(),"there is no remark of that index")
	CopyRemark(chain->m_oFcmReport.m_vRemarks[index],out,capacity);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}
