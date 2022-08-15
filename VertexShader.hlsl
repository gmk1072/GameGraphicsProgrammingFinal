#include "Vertex.hlsli"

// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix inverseTransposeWorld;
	// pass world inverse transpose here for correct normal transformation
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// The vertex's position (input.position) must be converted to world space,
	// then camera space (relative to our 3D camera), then to proper homogenous 
	// screen-space coordinates.  This is taken care of by our world, view and
	// projection matrices.  
	//
	// First we multiply them together to get a single matrix which represents
	// all of those transformations (world to view to projection space)
	matrix worldViewProj = mul(mul(world, view), projection);

	// Then we convert our 3-component position vector to a 4-component vector
	// and multiply it by our final 4x4 matrix.
	//
	// The result is essentially the position (XY) of the vertex on our 2D 
	// screen and the distance (Z) from the camera (the "depth" of the pixel)
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	// Send world position
	// get final WORLD position of the vertex
	output.worldPos = (float3)mul(float4(input.position, 1.0f), world);

	// Send normal through pipeline (for interpolation by rasterizer)
	// after transforming it!
	// Get top left of world matrix, keeps scale and rotation though!
	// We dont want to translate the normal, just scale and rotate
	// NOTE/WARNING/ALERT/REMINDER: THIS METHOD ONLY WORKS FOR UNIFORM SCALING!
	// To correctly scale normals for non-uniform scaling, pass in INVERSE TPOSE
	output.normal = mul(input.normal, (float3x3)inverseTransposeWorld);
	output.tangent = mul(input.tangent, (float3x3)inverseTransposeWorld);

	// Interpolate UV coordinates
	output.uv = input.uv;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}