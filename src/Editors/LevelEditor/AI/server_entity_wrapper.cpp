////////////////////////////////////////////////////////////////////////////
//	Module 		: server_entity_wrapper.cpp
//	Created 	: 16.10.2004
//  Modified 	: 16.10.2004
//	Author		: Dmitriy Iassenev
//	Description : Server entity wrapper
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server_entity_wrapper.h"
#include "..\xrServerEntities\xrServer_Objects.h"
#include "..\xrServerEntities\xrmessages.h"

struct ISE_Abstract;

CServerEntityWrapper::~CServerEntityWrapper	()
{
}

void CServerEntityWrapper::save(IWriter& stream)
{
	NET_Packet				net_packet;

	// Spawn
	stream.open_chunk(0);

	m_object->Spawn_Write(net_packet, TRUE);
	stream.w_u16(u16(net_packet.B.count));
	stream.w(net_packet.B.data, net_packet.B.count);

	stream.close_chunk();

	// Update
	stream.open_chunk(1);

	net_packet.w_begin(M_UPDATE);
	m_object->UPDATE_Write(net_packet);
	stream.w_u16(u16(net_packet.B.count));
	stream.w(net_packet.B.data, net_packet.B.count);

	stream.close_chunk();
}

void CServerEntityWrapper::load				(IReader &stream)
{
	R_ASSERT(false);
}

void CServerEntityWrapper::save_update		(IWriter &stream)
{
}

void CServerEntityWrapper::load_update		(IReader &stream)
{
}
