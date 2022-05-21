// Copyright Peter Leontev

#include "CSEdGraph.h"
#include "ComputeShader.h"

UComputeShader* UComputeShaderEdGraph::GetComputeShader() const
{
    return CastChecked<UComputeShader>(GetOuter());
}
