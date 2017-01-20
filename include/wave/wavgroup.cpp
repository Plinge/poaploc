#include "wavgroup.h"
#include "wave/sparsewave.h"
#include "wave/floatwave.h"

using namespace Audio;

//void WavGroup::CloneZero(AP_ArrayPtr<Wave> & other)
//{
//    SetSize(other.GetCount());
//    for (int i=0;i<other.GetCount();i++) {

//        Wave* w = other.GetAt(i);
//        if (w) {
//            Wave* m = GetAt(i);
//            if (!m) {
//                m = new FloatWave(w->getSamplingFrequency(),w->getChannels(),w->getSampleCount());
//                SetAt(i, m);
//            } else {
//                m->create(w->getSamplingFrequency(),w->getChannels(),w->getSampleCount());
//            }
//        } else {
//            SetAt(i,0);
//        }
//    }
//}

void WavGroup::clone(QVector< QSharedPointer< Wave > > & other, int typehint)
{
    clear();
    resize(other.size());
    for (int i=0;i<other.count();i++) {
        Wave* w = other.at(i).data();
        if (w) {
			if (typehint == WAVE_TYPE_SPARSEWAVE) {							
				setAt(i,new SparseWave(*w));
			} else {
				setAt(i,new FloatWave(*w));
			}
        } else {
            setAt(i,0);
        }
    }
}

void WavGroup::cloneZero(QVector< QSharedPointer< Wave > > & other, int typehint)
{
    clear();
    resize(other.size());
    for (int i=0;i<other.count();i++) {
        Wave* w = other.at(i).data();
        if (w) {
            if (typehint == WAVE_TYPE_SPARSEWAVE) {
                setAt(i,new SparseWave(w->getSamplingFrequency(),w->getChannels(),w->getFrameCount()));
            } else {
                setAt(i,new FloatWave(w->getSamplingFrequency(),w->getChannels(),w->getFrameCount()));
            }
        } else {
            setAt(i,0);
        }
    }
}

