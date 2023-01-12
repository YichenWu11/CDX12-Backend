#include <CDX12/Geometry/MeshMngr.h>
#include <CDX12/Geometry/GeometryGenerator.h>
#include <CDX12/Math/MathHelper.h>
#include <CDX12/DXUtil.h>

using namespace Chen::CDX12;
using namespace DirectX;

MeshMngr::MeshMngr(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) : device(device), cmdList(cmdList)
{
    this->BuildBasicGeo();
}

void MeshMngr::BuildBasicGeo()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset; 

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	XMFLOAT3 vMinf3(+Math::MathHelper::Infinity, +Math::MathHelper::Infinity, +Math::MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-Math::MathHelper::Infinity, -Math::MathHelper::Infinity, -Math::MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;

		XMVECTOR P = XMLoadFloat3(&box.Vertices[i].Position);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	// FIX: BoundingBox Error
	BoundingBox box_bounds;
	XMStoreFloat3(&box_bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&box_bounds.Extents, 0.5f * (vMax - vMin));
	boxSubmesh.Bounds = box_bounds;

	vMin = XMLoadFloat3(&vMinf3);
	vMax = XMLoadFloat3(&vMaxf3);
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;

		XMVECTOR P = XMLoadFloat3(&grid.Vertices[i].Position);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox grid_bounds;
	XMStoreFloat3(&grid_bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&grid_bounds.Extents, XMVectorSet(0.0f,0.0f,0.0f, 0.0f));
	// XMStoreFloat3(&grid_bounds.Extents, 0.5f * (vMax - vMin));
	gridSubmesh.Bounds = grid_bounds;

	vMin = XMLoadFloat3(&vMinf3);
	vMax = XMLoadFloat3(&vMaxf3);
	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;

		XMVECTOR P = XMLoadFloat3(&sphere.Vertices[i].Position);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox sphere_bounds;
	XMStoreFloat3(&sphere_bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&sphere_bounds.Extents, 0.5f * (vMax - vMin));
	sphereSubmesh.Bounds = sphere_bounds;

	vMin = XMLoadFloat3(&vMinf3);
	vMax = XMLoadFloat3(&vMaxf3);
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;

		XMVECTOR P = XMLoadFloat3(&cylinder.Vertices[i].Position);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox cylinder_bounds;
	XMStoreFloat3(&cylinder_bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&cylinder_bounds.Extents, 0.5f * (vMax - vMin));
	cylinderSubmesh.Bounds = cylinder_bounds;

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void MeshMngr::BuildTXTModelGeometryFromFile(
    const char* path, 
    const char* geoName, 
    const char* subName, 
    bool        is_normal, 
    bool        is_uv)
{
	std::ifstream fin(path);

	if (!fin)
	{
		MessageBox(0, L"file not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+Math::MathHelper::Infinity, +Math::MathHelper::Infinity, +Math::MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-Math::MathHelper::Infinity, -Math::MathHelper::Infinity, -Math::MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		if (is_normal) fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		else vertices[i].Normal = vertices[i].Pos;

		if (is_uv) {
			fin >> vertices[i].TexC.x >> vertices[i].TexC.y;
			vertices[i].TexC.y = 1.0f - vertices[i].TexC.y;
		}
		else vertices[i].TexC = { vertices[i].Pos.x, vertices[i].Pos.y };

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		XMVECTOR N = XMLoadFloat3(&vertices[i].Normal);

		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
		{
			XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
			XMStoreFloat3(&vertices[i].TangentU, T);
		}
		else
		{
			up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
			XMStoreFloat3(&vertices[i].TangentU, T);
		}

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	BoundingBox bounds;
	XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	submesh.Bounds = bounds;

	geo->DrawArgs[subName] = submesh;

	mGeometries[geo->Name] = std::move(geo);
}


void MeshMngr::BuildOBJModelGeometryFromFile(
	const char* path, 
	const char* geoName)
{
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./"; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(std::string(path), reader_config)) 
		if (!reader.Error().empty()) return;

	if (!reader.Warning().empty())
		OutputDebugString(AnsiToWString(reader.Warning()).c_str());

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	UINT vcount = 0;  // vertex count
	UINT tcount = 0;  // triangle(face) count
	for (size_t s = 0; s < shapes.size(); s++) tcount += shapes[s].mesh.indices.size();
	
	std::vector<Vertex> vertices(tcount);
	std::vector<std::int32_t> indices(tcount);
	
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = geoName;

	// Loop over shapes
	int idxInVertices = 0;
	for (size_t s = 0; s < shapes.size(); s++) {
		BoundingBox bounds;

		XMFLOAT3 vMinf3(+Math::MathHelper::Infinity, +Math::MathHelper::Infinity, +Math::MathHelper::Infinity);
		XMFLOAT3 vMaxf3(-Math::MathHelper::Infinity, -Math::MathHelper::Infinity, -Math::MathHelper::Infinity);

		XMVECTOR vMin = XMLoadFloat3(&vMinf3);
		XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices[idxInVertices].Pos.x = attrib.vertices[3*size_t(idx.vertex_index)+0];
				vertices[idxInVertices].Pos.y = attrib.vertices[3*size_t(idx.vertex_index)+1];
				vertices[idxInVertices].Pos.z = attrib.vertices[3*size_t(idx.vertex_index)+2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					vertices[idxInVertices].Normal.x = attrib.normals[3*size_t(idx.normal_index)+0];
					vertices[idxInVertices].Normal.y = attrib.normals[3*size_t(idx.normal_index)+1];
					vertices[idxInVertices].Normal.z = attrib.normals[3*size_t(idx.normal_index)+2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					vertices[idxInVertices].TexC.x = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
					vertices[idxInVertices].TexC.y = 1.0f - attrib.texcoords[2*size_t(idx.texcoord_index)+1];
				}

				XMVECTOR P = XMLoadFloat3(&vertices[idxInVertices].Pos);

				XMVECTOR N = XMLoadFloat3(&vertices[idxInVertices].Normal);

				XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
				if (fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
				{
					XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
					XMStoreFloat3(&vertices[idxInVertices].TangentU, T);
				}
				else
				{
					up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
					XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
					XMStoreFloat3(&vertices[idxInVertices].TangentU, T);
				}

				vMin = XMVectorMin(vMin, P);
				vMax = XMVectorMax(vMax, P);

				++idxInVertices;
			}
			index_offset += fv;
		}
		XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
		XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));

		SubmeshGeometry submesh;
		submesh.IndexCount = (UINT)shapes[s].mesh.indices.size();
		if (s == 0)
		{
			submesh.StartIndexLocation = 0;
			submesh.BaseVertexLocation = 0;
		}
		else
		{
			submesh.StartIndexLocation = shapes[s-1].mesh.indices.size();
			submesh.BaseVertexLocation = shapes[s-1].mesh.indices.size();			
		}

		submesh.Bounds = bounds;

		geo->DrawArgs[shapes[s].name] = submesh;
	}

	for (UINT i = 0; i < tcount; ++i)
	{
		indices[i] = i;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = DXUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	mGeometries[geo->Name] = std::move(geo);
}

