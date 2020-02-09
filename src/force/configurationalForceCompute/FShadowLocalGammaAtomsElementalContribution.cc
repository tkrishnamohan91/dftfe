// ---------------------------------------------------------------------
//
// Copyright (c) 2017-2020 The Regents of the University of Michigan and DFT-FE authors.
//
// This file is part of the DFT-FE code.
//
// The DFT-FE code is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the DFT-FE distribution.
//
// ---------------------------------------------------------------------
//
// @author Sambit Das
//

//(locally used function) compute FShadowLocal contibution due to Gamma(Rj) for given set of cells
template<unsigned int FEOrder>
void forceClass<FEOrder>::FShadowLocalGammaAtomsElementalContribution
             (std::map<unsigned int, std::vector<double> > & forceContributionLocalGammaAtoms,
	      FEEvaluation<C_DIM,1,C_num1DQuad<FEOrder>(),C_DIM>  & forceEval,
              const MatrixFree<3,double> & matrixFreeData,
	      const unsigned int cell,
              const std::map<unsigned int,std::map<dealii::CellId, std::vector<double> > > & gradRhoAtomsQuads,
              const std::vector< VectorizedArray<double> > & derVxcWithRhoOutTimesRhoDiffQuads,
              const std::vector< VectorizedArray<double> > & phiRhoMinusApproxRhoQuads)
{
  Tensor<1,C_DIM,VectorizedArray<double> > zeroTensor1;
  for (unsigned int idim=0; idim<C_DIM; idim++)
    zeroTensor1[idim]=make_vectorized_array(0.0);
  const unsigned int numberGlobalAtoms = dftPtr->atomLocations.size();
  const unsigned int numberImageCharges = dftPtr->d_imageIdsTrunc.size();
  const unsigned int totalNumberAtoms = numberGlobalAtoms + numberImageCharges;
  const unsigned int numSubCells= matrixFreeData.n_components_filled(cell);
  const unsigned int numQuadPoints=forceEval.n_q_points;
  DoFHandler<C_DIM>::active_cell_iterator subCellPtr;

  for (unsigned int iAtom=0;iAtom <totalNumberAtoms; iAtom++)
  {
    std::vector<Tensor<1,C_DIM,VectorizedArray<double> > > gradRhoQuads(numQuadPoints,zeroTensor1);

    double atomCharge;
    unsigned int atomId=iAtom;
    Point<C_DIM> atomLocation;
    if(iAtom < numberGlobalAtoms)
    {
       atomLocation[0]=dftPtr->atomLocations[iAtom][2];
       atomLocation[1]=dftPtr->atomLocations[iAtom][3];
       atomLocation[2]=dftPtr->atomLocations[iAtom][4];
       if(dftParameters::isPseudopotential)
         atomCharge = dftPtr->atomLocations[iAtom][1];
       else
         atomCharge = dftPtr->atomLocations[iAtom][0];
    }
    else
    {
       const int imageId=iAtom-numberGlobalAtoms;
       atomId=dftPtr->d_imageIdsTrunc[imageId];
       atomCharge = dftPtr->d_imageChargesTrunc[imageId];
       atomLocation[0]=dftPtr->d_imagePositionsTrunc[imageId][0];
       atomLocation[1]=dftPtr->d_imagePositionsTrunc[imageId][1];
       atomLocation[2]=dftPtr->d_imagePositionsTrunc[imageId][2];
    }

    if (gradRhoAtomsQuads.find(iAtom)==gradRhoAtomsQuads.end())
       continue;

    for (unsigned int iSubCell=0; iSubCell<numSubCells; ++iSubCell)
    {
       subCellPtr= matrixFreeData.get_cell_iterator(cell,iSubCell);
       dealii::CellId subCellId=subCellPtr->id();

       std::map<dealii::CellId, std::vector<double> >::const_iterator it
	      =gradRhoAtomsQuads.find(iAtom)->second.find(subCellId);
       if (it!=gradRhoAtomsQuads.find(iAtom)->second.end())
	    for (unsigned int q=0; q<numQuadPoints; ++q)
	    {
	       gradRhoQuads[q][0][iSubCell]=(it->second)[q*C_DIM];
	       gradRhoQuads[q][1][iSubCell]=(it->second)[q*C_DIM+1];
	       gradRhoQuads[q][2][iSubCell]=(it->second)[q*C_DIM+2];
	    }

    }//subCell loop

    for (unsigned int q=0; q<numQuadPoints; ++q)
      forceEval.submit_value(-gradRhoQuads[q]*(derVxcWithRhoOutTimesRhoDiffQuads[q]+phiRhoMinusApproxRhoQuads[q]),q);
    
    Tensor<1,C_DIM,VectorizedArray<double> > forceContributionLocalGammaiAtomCells
						 =forceEval.integrate_value();

    if (forceContributionLocalGammaAtoms.find(atomId)==forceContributionLocalGammaAtoms.end())
       forceContributionLocalGammaAtoms[atomId]=std::vector<double>(C_DIM,0.0);

    for (unsigned int iSubCell=0; iSubCell<numSubCells; ++iSubCell)
       for (unsigned int idim=0; idim<C_DIM; idim++)
         forceContributionLocalGammaAtoms[atomId][idim]+=
	       forceContributionLocalGammaiAtomCells[idim][iSubCell];
  }//iAtom loop
}