struct VertexOut{
	float4 position : SV_Position
};

[shader("vertex")]
VertexOut vsMain(uint vertexID : SV_VertexID){
	float2 positions[3] = {
		float2(0.0,-0.5),
		float2(0.5, 0.5),
		float2(-0.5, 0.5)
	};
	
	VertexOut output;
	output.position =float4(positions[vertexID], 0.0, 1.0);
	return output;
}

[shader("pixel")]
float4 psMain(VertexOut input) : SV_Target{
	return float4(0.0, 1.0, 1.0, 1.0);
}