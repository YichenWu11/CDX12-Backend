#pragma once

#include "CDX12/CmdListHandle.h"
#include "CDX12/CmdQueue.h"
#include "CDX12/DXUtil.h"
#include "CDX12/DescriptorHeapMngr.h"
#include "CDX12/Device.h"
#include "CDX12/FrameResourceMngr.h"
#include "CDX12/GCmdList.h"
#include "CDX12/Math/MathHelper.h"
#include "CDX12/Math/Quaternion.h"
#include "CDX12/Metalib.h"
#include "CDX12/Resource//DescHeapAllocView.h"
#include "CDX12/Resource/BufferView.h"
#include "CDX12/Resource/DefaultBuffer.h"
#include "CDX12/Resource/Mesh.h"
#include "CDX12/Resource/ReadbackBuffer.h"
#include "CDX12/Resource/ResourceStateTracker.h"
#include "CDX12/Resource/Texture.h"
#include "CDX12/Resource/UploadBuffer.h"
#include "CDX12/Shader/BasicShader.h"
#include "CDX12/Shader/PSOManager.h"
#include "CDX12/VarSizeAllocMngr.h"

#include "CDX12/_deps/d3dx12.h"