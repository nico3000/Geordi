#include "StdAfx.h"
#include "BaseSceneNode.h"
#include "GraphicsLayer.h"


HRESULT BaseSceneNode::VOnLostDevice(void)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        HRESULT hr = (*iter)->VOnLostDevice();
        RETURN_IF_FAILED(hr);
    }
    return S_OK;
}


HRESULT BaseSceneNode::VOnRestore(void)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        HRESULT hr = (*iter)->VOnRestore();
        RETURN_IF_FAILED(hr);
    }
    return S_OK;
}


HRESULT BaseSceneNode::VRenderChildren(void)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        HRESULT hr = S_OK;
        hr = (*iter)->VPreRender();
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VRender();
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VRenderChildren();
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VPostRender();
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
    }
}

bool BaseSceneNode::VAddChild(std::shared_ptr<ISceneNode> p_pChild)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        if((*iter) == p_pChild)
        {
            LI_ERROR("duplicate child");
            return false;
        }
    }
    m_children.push_back(p_pChild);
    return true;
}

bool BaseSceneNode::VRemoveChild(ActorID p_actorID)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        std::shared_ptr<BaseSceneNode> pNode = std::static_pointer_cast<BaseSceneNode>(*iter);
        if(pNode)
        {
            StrongActorPtr pActor = pNode->m_pActor.lock();
            if(pActor && pActor->GetID() == p_actorID)
            {
                m_children.erase(iter);
                return true;
            }
        }
    }
    return false;
}
