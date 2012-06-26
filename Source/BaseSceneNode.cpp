#include "StdAfx.h"
#include "BaseSceneNode.h"
#include "GraphicsLayer.h"


BaseSceneNode::BaseSceneNode(ActorID p_actorID) :
m_isVisible(true)
{
    m_properties.m_actorID = p_actorID;
}


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


HRESULT BaseSceneNode::VRenderChildren(Scene* p_pScene)
{
    for(auto iter=m_children.begin(); iter != m_children.end(); ++iter)
    {
        HRESULT hr = S_OK;
        hr = (*iter)->VPreRender(p_pScene);
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VRender(p_pScene);
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VRenderChildren(p_pScene);
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
        hr = (*iter)->VPostRender(p_pScene);
        if(FAILED(hr))
        {
            HRESULT_TO_WARNING(hr);
            continue;
        }
    }
    return S_OK;
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
            if(pNode->m_properties.GetActorID() == p_actorID)
            {
                m_children.erase(iter);
                return true;
            }
        }
    }
    return false;
}
