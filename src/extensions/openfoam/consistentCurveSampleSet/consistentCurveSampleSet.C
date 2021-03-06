/*---------------------------------------------------------------------------* \
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "consistentCurveSampleSet.H"
#include "meshSearch.H"
#include "DynamicList.H"
#include "polyMesh.H"

#ifdef OF16ext
#ifdef Fx40
#include "CloudTemplate.H"
#else
#include "Cloud.H"
#endif
#include "passiveParticle.H"
#include "IDLList.H"
#endif

#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(consistentCurveSet, 0);
    addToRunTimeSelectionTable(sampledSet, consistentCurveSet, word);
    
#if defined(OFplus)||defined(OFdev)||defined(OFesi1806)
    const scalar consistentCurveSet::tol = 1e-3;
#endif
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

#ifdef CONSISTENTCURVESAMPLESET_V1
// Sample till hits boundary.
bool Foam::consistentCurveSet::trackToBoundary
(
#ifdef OF16ext
            Particle<passiveParticle>& singleParticle,
#elif defined (OF21x)||defined (OF22x)||defined (OF22eng)
            passiveParticle& singleParticle,
#else
            passiveParticleCloud& particleCloud,
            passiveParticle& singleParticle,
#endif
    label& sampleI,
    DynamicList<point>& samplingPts,
    DynamicList<label>& samplingCells,
    DynamicList<label>& samplingFaces,
    DynamicList<scalar>& samplingCurveDist
) const
{
#ifdef OF16ext
#elif defined(OF21x)||defined(OF22x)||defined (OF22eng)

    passiveParticleCloud particleCloud(mesh());
    particle::TrackingData<passiveParticleCloud> trackData(particleCloud);

#elif not defined(OFdev)
    particle::TrackingData<passiveParticleCloud> trackData(particleCloud);
#endif


#if not defined(OFdev)
    // Alias
    const point& trackPt = singleParticle.position();
#endif


    while(true)
    {
        // Local geometry info
        const vector offset = sampleCoords_[sampleI+1] - sampleCoords_[sampleI];
        const scalar smallDist = mag(tol*offset);

#if not defined(OFdev)
        point oldPos = trackPt;
        label facei = -1;
        do
        {
            singleParticle.stepFraction() = 0;
#ifdef OF16ext
            singleParticle.track(sampleCoords_[sampleI+1]);
#else
            singleParticle.track(sampleCoords_[sampleI+1], trackData);
#endif
        }
        while
        (
            !singleParticle.onBoundary()
         && (mag(trackPt - oldPos) < smallDist)
        );
#else
        singleParticle.track(offset, 0);
        const point trackPt = singleParticle.position();
#endif

        if (
#if defined(OFdev)
            singleParticle.onBoundaryFace()
#else
            singleParticle.onBoundary()
#endif
            )
        {
            //Info<< "trackToBoundary : reached boundary"
            //    << "  trackPt:" << trackPt << endl;
            if
            (
                mag(trackPt - sampleCoords_[sampleI+1])
              < smallDist
            )
            {
                // Reached samplePt on boundary
                //Info<< "trackToBoundary : boundary. also sampling."
                //    << "  trackPt:" << trackPt << " sampleI+1:" << sampleI+1
                //    << endl;
                samplingPts.append(trackPt);
                samplingCells.append(singleParticle.cell());
                samplingFaces.append(
      #if defined(OFdev)
                      singleParticle.face()
      #else
                      facei
      #endif
                      );

                // trackPt is at sampleI+1
                samplingCurveDist.append(1.0*(sampleI+1));
            }
            return true;
        }

        // Reached samplePt in cell
        samplingPts.append(trackPt);
        samplingCells.append(singleParticle.cell());
        samplingFaces.append(-1);

        // Convert trackPt to fraction in between sampleI and sampleI+1
        scalar dist =
            mag(trackPt - sampleCoords_[sampleI])
          / mag(sampleCoords_[sampleI+1] - sampleCoords_[sampleI]);
        samplingCurveDist.append(sampleI + dist);

        // go to next samplePt
        sampleI++;

        if (sampleI == sampleCoords_.size() - 1)
        {
            // no more samples.
            //Info<< "trackToBoundary : Reached end : sampleI now:" << sampleI
            //    << endl;
            return false;
        }
    }
}

void Foam::consistentCurveSet::calcSamples
(
    DynamicList<point>& samplingPts,
    DynamicList<label>& samplingCells,
    DynamicList<label>& samplingFaces,
    DynamicList<label>& samplingSegments,
    DynamicList<scalar>& samplingCurveDist
) const
{
    // Check sampling points
    if (sampleCoords_.size() < 2)
    {
        FatalErrorIn("consistentCurveSet::calcSamples()")
            << "Incorrect sample specification. Too few points:"
            << sampleCoords_ << exit(FatalError);
    }
    point oldPoint = sampleCoords_[0];
    for(label sampleI = 1; sampleI < sampleCoords_.size(); sampleI++)
    {
        if (mag(sampleCoords_[sampleI] - oldPoint) < SMALL)
        {
            FatalErrorIn("consistentCurveSet::calcSamples()")
                << "Incorrect sample specification."
                << " Point " << sampleCoords_[sampleI-1]
                << " at position " << sampleI-1
                << " and point " << sampleCoords_[sampleI]
                << " at position " << sampleI
                << " are too close" << exit(FatalError);
        }
        oldPoint = sampleCoords_[sampleI];
    }

#ifdef OF16ext
#else
    // Force calculation of cloud addressing on all processors
    const bool oldMoving = const_cast<polyMesh&>(mesh()).moving(false);
    passiveParticleCloud particleCloud(mesh());
#endif

    // current segment number
    label segmentI = 0;

    // starting index of current segment in samplePts
    label startSegmentI = 0;

    label sampleI = 0;

    point lastSample(GREAT, GREAT, GREAT);
//    scalar totaldist=0.0;
    while (true)
    {
        // Get boundary intersection
        point trackPt;
        label trackCellI = -1;
        label trackFaceI = -1;

        do
        {
            const vector offset =
                sampleCoords_[sampleI+1] - sampleCoords_[sampleI];
            const scalar smallDist = mag(tol*offset);


            // Get all boundary intersections
            List<pointIndexHit> bHits = searchEngine().intersections
            (
                sampleCoords_[sampleI],
                sampleCoords_[sampleI + 1]
            );

            point bPoint(GREAT, GREAT, GREAT);
            label bFaceI = -1;

            if (bHits.size())
            {
                bPoint = bHits[0].hitPoint();
                bFaceI = bHits[0].index();
            }

            // Get tracking point

            bool isSample =
                getTrackingPoint
                (
#if not (defined(OFplus)||defined(OFdev))
                    sampleCoords_[sampleI+1] - sampleCoords_[sampleI],
#endif
                    sampleCoords_[sampleI],
                    bPoint,
                    bFaceI,
#if defined(OFplus)||defined(OFdev)
                    smallDist,
#endif

                    trackPt,
                    trackCellI,
                    trackFaceI
                );

            if (isSample && (mag(lastSample - trackPt) > smallDist))
            {
                //Info<< "calcSamples : getTrackingPoint returned valid sample "
                //    << "  trackPt:" << trackPt
                //    << "  trackFaceI:" << trackFaceI
                //    << "  trackCellI:" << trackCellI
                //    << "  sampleI:" << sampleI
                //    << "  dist:" << dist
                //    << endl;

                samplingPts.append(trackPt);
                samplingCells.append(trackCellI);
                samplingFaces.append(trackFaceI);

                // Convert sampling position to unique curve parameter. Get
                // fraction of distance between sampleI and sampleI+1.
                scalar dist =
                    mag(trackPt - sampleCoords_[sampleI])
                  / mag(sampleCoords_[sampleI+1] - sampleCoords_[sampleI]);
                samplingCurveDist.append(sampleI + dist);
		
// 		if (samplingPts.size()>0) totaldist+=mag(trackPt-lastSample);
//                 samplingCurveDist.append(totaldist);

                lastSample = trackPt;
            }

            if (trackCellI == -1)
            {
                // No intersection found. Go to next point
                sampleI++;
            }
        } while ((trackCellI == -1) && (sampleI < sampleCoords_.size() - 1));

        if (sampleI == sampleCoords_.size() - 1)
        {
            //Info<< "calcSamples : Reached end of samples: "
            //    << "  sampleI now:" << sampleI
            //    << endl;
            break;
        }

        //
        // Segment sampleI .. sampleI+1 intersected by domain
        //

#ifdef OF16ext
        // Initialize tracking starting from sampleI
        Cloud<passiveParticle> particles(mesh(), IDLList<passiveParticle>());

        passiveParticle singleParticle
        (
            particles,
            trackPt,
            trackCellI
        );
#else
        // Initialize tracking starting from sampleI
        passiveParticle singleParticle
        (
            mesh(),
            trackPt,
            trackCellI
        );
#endif

        bool bReached = trackToBoundary
        (
#if defined(OF23x)||defined(OF301)||defined (OFplus)||defined(OFdev)
            particleCloud,
#endif
            singleParticle,
            sampleI,
            samplingPts,
            samplingCells,
            samplingFaces,
            samplingCurveDist
        );

        // fill sampleSegments
        for (label i = samplingPts.size() - 1; i >= startSegmentI; --i)
        {
            samplingSegments.append(segmentI);
        }

        if (!bReached)
        {
            //Info<< "calcSamples : Reached end of samples: "
            //    << "  sampleI now:" << sampleI
            //    << endl;
            break;
        }
        lastSample = singleParticle.position();


        // Find next boundary.
        sampleI++;

        if (sampleI == sampleCoords_.size() - 1)
        {
            //Info<< "calcSamples : Reached end of samples: "
            //    << "  sampleI now:" << sampleI
            //    << endl;
            break;
        }

        segmentI++;

        startSegmentI = samplingPts.size();
    }
#ifdef OF16ext
#else
    const_cast<polyMesh&>(mesh()).moving(oldMoving);
#endif
}
#else
void Foam::consistentCurveSet::calcSamples
(
    DynamicList<point>& samplingPts,
    DynamicList<label>& samplingCells,
    DynamicList<label>& samplingFaces,
    DynamicList<label>& samplingSegments,
    DynamicList<scalar>& samplingCurveDist
) const
{
    const meshSearch& queryMesh = searchEngine();

    labelList foundProc(sampleCoords_.size(), -1);
    forAll(sampleCoords_, sampleI)
    {
        label celli = queryMesh.findCell(sampleCoords_[sampleI]);

        if (celli != -1)
        {
            samplingPts.append(sampleCoords_[sampleI]);
            samplingCells.append(celli);
            samplingFaces.append(-1);
            samplingSegments.append(0);
            samplingCurveDist.append(1.0 * (sampleI+1));

            foundProc[sampleI] = Pstream::myProcNo();
        }
    }

    // Check that all have been found
    labelList maxFoundProc(foundProc);
    Pstream::listCombineGather(maxFoundProc, maxEqOp<label>());
    Pstream::listCombineScatter(maxFoundProc);

    labelList minFoundProc(foundProc.size(), labelMax);
    forAll(foundProc, i)
    {
        if (foundProc[i] != -1)
        {
            minFoundProc[i] = foundProc[i];
        }
    }
    Pstream::listCombineGather(minFoundProc, minEqOp<label>());
    Pstream::listCombineScatter(minFoundProc);


    DynamicList<point> missingPoints(sampleCoords_.size());

    forAll(sampleCoords_, sampleI)
    {
        if (maxFoundProc[sampleI] == -1)
        {
            // No processor has found the location.
            missingPoints.append(sampleCoords_[sampleI]);
        }
        else if (minFoundProc[sampleI] != maxFoundProc[sampleI])
        {
            WarningIn("calcSamples")
                << "For sample set " << name()
                << " location " << sampleCoords_[sampleI]
                << " seems to be on multiple domains: "
                << minFoundProc[sampleI] << " and " << maxFoundProc[sampleI]
                << nl
                << "This might happen if the location is on"
                << " a processor patch. Change the location slightly"
                << " to prevent this." << endl;
        }
    }


    if (missingPoints.size() > 0)
    {
        if (missingPoints.size() < 100 || debug)
        {
            WarningIn("calcSamples")
                << "For sample set " << name()
                << " did not found " << missingPoints.size()
                << " points out of " << sampleCoords_.size()
                << nl
                << "Missing points:" << missingPoints << endl;
        }
        else
        {
            WarningIn("calcSamples")
                << "For sample set " << name()
                << " did not found " << missingPoints.size()
                << " points out of " << sampleCoords_.size()
                << nl
                << "Print missing points by setting the debug flag"
                << " for " << consistentCurveSet::typeName << endl;
        }
    }

}
#endif

void Foam::consistentCurveSet::genSamples()
{
    // Storage for sample points
    DynamicList<point> samplingPts;
    DynamicList<label> samplingCells;
    DynamicList<label> samplingFaces;
    DynamicList<label> samplingSegments;
    DynamicList<scalar> samplingCurveDist;

    calcSamples
    (
        samplingPts,
        samplingCells,
        samplingFaces,
        samplingSegments,
        samplingCurveDist
    );

    samplingPts.shrink();
    samplingCells.shrink();
    samplingFaces.shrink();
    samplingSegments.shrink();
    samplingCurveDist.shrink();

    DynamicList<scalar> delta;
    if (samplingPts.size()>0) delta.append(0.0);
    for(int i=1; i<samplingPts.size();i++)
      delta.append(delta[i-1]+mag(samplingPts[i]-samplingPts[i-1]));
    delta.shrink();
    
    setSamples
    (
        samplingPts,
        samplingCells,
        samplingFaces,
        samplingSegments,
#ifdef OF16ext
        samplingCurveDist
#else
// 	mag(samplingPts-basept_)
	delta
#endif
    );
    
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::consistentCurveSet::consistentCurveSet
(
    const word& name,
    const polyMesh& mesh,
#if defined(OF16ext)||defined(OF21x)
    meshSearch& searchEngine,
#else
    const meshSearch& searchEngine,
#endif
    const word& axis,
    const List<point>& sampleCoords
)
:
    sampledSet(name, mesh, searchEngine, axis),
    sampleCoords_(sampleCoords),
    basept_(sampleCoords_[0])
{
    genSamples();

    if (debug)
    {
        write(Info);
    }
}


Foam::consistentCurveSet::consistentCurveSet
(
    const word& name,
    const polyMesh& mesh,
#if defined(OF16ext)||defined(OF21x)
    meshSearch& searchEngine,
#else
    const meshSearch& searchEngine,
#endif
    const dictionary& dict
)
:
    sampledSet(name, mesh, searchEngine, dict),
    sampleCoords_(dict.lookup("points")),
    basept_(sampleCoords_[0])
{
    genSamples();

    if (debug)
    {
        write(Info);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::consistentCurveSet::~consistentCurveSet()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::point Foam::consistentCurveSet::getRefPoint(const List<point>& pts) const
{
    if (pts.size())
    {
        // Use first samplePt as starting point
        return /*pts[0]*/basept_;
    }
    else
    {
        return vector::zero;
    }
}


// ************************************************************************* //
