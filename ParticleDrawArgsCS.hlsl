// ALL BASED ON http://twvideo01.ubm-us.net/o1/vault/GDC2014/Presentations/Gareth_Thomas_Compute-based_GPU_Particle.pdf
// WITH HELP FROM CHRIS Cascioli
// RESET COUNTERS EACH DRAW HERE https://www.gamedev.net/topic/662293-reset-hidden-counter-of-structuredappendconsume-buffers/
// CLEAR ALIVE LIST EACH DRAW
// Matching args found from https://msdn.microsoft.com/en-us/library/windows/desktop/ff476410(v=vs.85).aspx

// Numbe of alive particles, copied via CopyStructureCount
cbuffer numAliveParticles : register(b0)
{
    uint numAliveParticles; // number of dead particles, prevent overconsumption
    uint3 padding; // padding for 16byte alignment 
};

RWByteAddressBuffer drawArgs : register(u0); // Holds draw arguments on GPU for indirect draw

// Simple compute shader that runs one time which will store relevant info
// for the DrawIndexedInstancedIndirect command
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{   
    drawArgs.Store(0, 6); // Unsure if this is number of indices per particle OR the current particle to be drawn?
    drawArgs.Store(4, numAliveParticles);
    drawArgs.Store(8, 0);
    drawArgs.Store(12, 0);
    drawArgs.Store(16, 0);
}