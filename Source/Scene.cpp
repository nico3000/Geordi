#include "StdAfx.h"
#include "Scene.h"


Scene::Scene(void) :
m_pRoot(0)
{
    m_pRoot.reset(new RootNode);
    m_pCurrentCamera.reset(new Camera);
    if(FAILED(m_pCurrentCamera->Init()))
    {
        m_pCurrentCamera = std::shared_ptr<Camera>();
    }
}


Scene::~Scene(void)
{
}


HRESULT Scene::OnUpdate(unsigned long m_deltaMillis)
{
    return m_pRoot ? m_pRoot->VOnUpdate(this, m_deltaMillis) : S_OK;
}


HRESULT Scene::OnRestore(void)
{
    return m_pRoot ? m_pRoot->VOnRestore() : S_OK;
}


HRESULT Scene::OnLostDevice(void)
{
    return m_pRoot ? m_pRoot->VOnLostDevice() : S_OK;
}


void Scene::Render(void)
{
    if(m_pRoot && m_pCurrentCamera)
    {
        //LI_INFO("Scene::Render()");
        m_pCurrentCamera->Bind();
        m_pCurrentCamera->Update(Camera::MATRIX_BOTH);

        m_pRoot->VPreRender(this);
        m_pRoot->VRender(this);
        m_pRoot->VRenderChildren(this);
        m_pRoot->VPostRender(this);
    }
}


bool Scene::AddChild(ActorID p_actorID, std::shared_ptr<ISceneNode> p_pChild)
{
    return m_pRoot ? m_pRoot->VAddChild(p_pChild) : false;
}


bool Scene::RemoveChild(ActorID p_actorID)
{
    return m_pRoot ? m_pRoot->VRemoveChild(p_actorID) : false;
}