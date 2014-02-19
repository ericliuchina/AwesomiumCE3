/* Awesomium_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <CPluginAwesomium.h>
#include <IUIDraw.h>
#include <IEntitySystem.h>

namespace AwesomiumPlugin
{
    CPluginAwesomium* gPlugin = NULL;

    CPluginAwesomium::CPluginAwesomium() : m_bEnablePlugins( false ), m_bVisible( false )
    {
        gPlugin = this;
    }

    CPluginAwesomium::~CPluginAwesomium()
    {
        gPlugin = NULL;
    }


    bool CPluginAwesomium::Init( SSystemGlobalEnvironment& env, SSystemInitParams& startupParams, IPluginBase* pPluginManager, const char* sPluginDirectory )
    {
        gPluginManager = ( PluginManager::IPluginManager* )pPluginManager->GetConcreteInterface( NULL );
        CPluginBase::Init( env, startupParams, pPluginManager, sPluginDirectory );

        // Note: Autoregister Flownodes will be automatically registered

        InitAwesomium();

        return true;
    }

    const char* CPluginAwesomium::ListCVars() const
    {
        return "..."; // TODO: Enter CVARs/Commands here if you have some
    }

    const char* CPluginAwesomium::GetStatus() const
    {
        return "OK";
    }

    void MaterialCommand( IConsoleCmdArgs* pArgs )
    {
        auto entity = gEnv->pEntitySystem->FindEntityByName( pArgs->GetArg( 1 ) );

        if ( entity )
        {
            auto material = entity->GetMaterial();

            if ( material )
            {
                gEnv->pLog->LogAlways( "Found material %s", material->GetName() );
            }
        }
    }

    bool CPluginAwesomium::InitAwesomium()
    {
        if ( !gEnv || !gEnv->pGame->GetIGameFramework() )
        {
            gEnv->pLog->LogError( "Failed to initialize Awesomium, no gameframework found. This is probably caused by calling the Init method too soon" );
            return false;
        }

        IGameFramework* pGameFramework = gEnv->pGame->GetIGameFramework();
        pGameFramework->RegisterListener( this, "AwesomiumCE3", eFLPriority_Default );

        WebConfig config;
        config.remote_debugging_port = 3000;
        m_pWebCore = WebCore::Initialize( config );

        // Initialize DataPakSource
        m_DataSource = NULL;

        string sUIpak = gPluginManager->GetDirectoryGame();
        sUIpak += "\\UIFiles.pak";

        m_DataSource = new DataPakSource( WSLit( sUIpak ) );
        LoadElement( "UI/lowerleft.htm" );
        SetVisible( true );

        gEnv->pConsole->RegisterInt( "aw_t0", 1, 0 );
        gEnv->pConsole->RegisterInt( "aw_t1", 0, 0 );
        gEnv->pConsole->RegisterInt( "aw_s0", 0, 0 );
        gEnv->pConsole->RegisterInt( "aw_s1", 1, 0 );
        gEnv->pConsole->AddCommand( "aw_material", MaterialCommand );

        return true;

    }

    void CPluginAwesomium::Shutdown()
    {
    }

    void CPluginAwesomium::OnPostUpdate( float fDeltaTime )
    {
        if ( m_bVisible )
        {
            WebCore::instance()->Update();
            std::for_each( std::begin( m_uiElements ), std::end( m_uiElements ), [&]( std::shared_ptr<HTMLElement>& e )
            {
                if ( e->IsVisible() )
                {
                    e->OnUpdate();
                }
            } );
        }
    }

    void CPluginAwesomium::OnSaveGame( ISaveGame* pSaveGame )
    {

    }

    void CPluginAwesomium::OnLoadGame( ILoadGame* pLoadGame )
    {
    }

    void CPluginAwesomium::OnLevelEnd( const char* nextLevel )
    {
    }

    void CPluginAwesomium::OnActionEvent( const SActionEvent& event )
    {
    }

    void CPluginAwesomium::OnPreRender()
    {
    }

    void CPluginAwesomium::SetVisible( bool visible )
    {
        m_bVisible = visible;
    }

    bool CPluginAwesomium::IsVisible() const
    {
        return m_bVisible;
    }

    int CPluginAwesomium::LoadElement( const char* pathToHtml )
    {
        auto element = std::make_shared<HTMLElement>( pathToHtml );

        m_uiElements.push_back( element );

        return 0;
    }

    bool CPluginAwesomium::CheckDependencies() const
    {
        bool bRet = CPluginBase::CheckDependencies();

        return bRet;
    }

    bool CPluginAwesomium::Release( bool bForce )
    {
        bool bRet = true;

        if ( !m_bCanUnload )
        {
            // Should be called while Game is still active otherwise there might be leaks/problems
            bRet = CPluginBase::Release( bForce );

            if ( bRet )
            {
                // TODO: Do your own cleanups here
                // Cleanup like this always (since the class is static its cleaned up when the dll is unloaded)
                gPluginManager->UnloadPlugin( GetName() );

                // Allow Plugin Manager garbage collector to unload this plugin
                AllowDllUnload();
            }
        }

        return bRet;
    };

    bool CPluginAwesomium::InitDependencies()
    {
        return CPluginBase::InitDependencies();
    }
}