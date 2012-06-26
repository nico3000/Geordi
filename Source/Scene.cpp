#include "StdAfx.h"
#include "Scene.h"


Scene::Scene(void) :
m_pCurrentCamera(0)
{
    m_pRoot.reset(new RootNode);
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
        m_pCurrentCamera->Update(Camera::MATRIX_BOTH);
        m_pCurrentCamera->Bind();

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


void Scene::AddCamera(const std::string& p_name, std::shared_ptr<Camera> p_pCamera, bool p_activate /* = false */)
{
    m_cameras[p_name] = p_pCamera;
    if(p_activate || !m_pCurrentCamera)
    {
        m_pCurrentCamera = p_pCamera;
    }
}