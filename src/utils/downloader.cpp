#include "downloader.h"

#ifndef NO_TORRENT_SYSTEM

#ifdef _MSC_VER
#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX
#include <winsock2.h>
#endif // _MSC_VER


#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/convauto.h>
#include <wx/log.h>
#include <wx/tokenzr.h>

#include "customdialogs.h"//shoudl be remove d fater initlai debug
#include "conversion.h"
#include "debug.h"
#include "globalevents.h"
#include "../socket.h"
#include "../globalsmanager.h"
#include "../uiutils.h"
#include "conversion.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include "curlhelper.h"

const wxString s_soap_service_url = _T("http://zero-k.info/ContentService.asmx?op=DownloadFile");

const wxString s_soap_querytemplate = _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" \
"<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\n"\
"   <soap12:Body>\n"\
"       <DownloadFile xmlns=\"http://tempuri.org/\">\n"\
"           <internalName>REALNAME</internalName>\n"\
"       </DownloadFile>\n"\
"   </soap12:Body>\n"\
"</soap12:Envelope>\0");

const wxString s_soap_querytemplate_resourcelist = _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" \
"<soap12:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap12=\"http://www.w3.org/2003/05/soap-envelope\">\n"\
"   <soap12:Body>\n"\
"       <GetResourceList xmlns=\"http://tempuri.org/\" />\n"\
"   </soap12:Body>\n"\
"</soap12:Envelope>\0");

PlasmaInterface& plasmaInterface()
{
    static LineInfo<PlasmaInterface> m( AT );
	static GlobalObjectHolder<PlasmaInterface, LineInfo<PlasmaInterface> > m_plasma( m );
	return m_plasma;
}

/** @brief PlasmaInterface
  *
  * @todo: document this function
  */
PlasmaInterface::PlasmaInterface()
	: m_host ( _T("zero-k.info") ),
	m_remote_path ( _T("ContentService.asmx") )
{
	m_worker_thread.Create();
	m_worker_thread.SetPriority( WXTHREAD_MIN_PRIORITY );
	m_worker_thread.Run();
}

PlasmaInterface::~PlasmaInterface()
{
	m_worker_thread.Wait();
}

/** @brief GetResourceInfo
  *
  * @todo: document this function
  */
PlasmaResourceInfo PlasmaInterface::GetResourceInfo(const wxString& name)
{
    const int index =  -1 - m_buffers.size();
    wxString data = s_soap_querytemplate;
    data.Replace( _T("REALNAME") , name );

    wxStringInputStream req ( data );
    wxStringOutputStream response;
    wxStringOutputStream rheader;
    CURL *curl_handle;
    curl_handle = curl_easy_init();
    struct curl_slist* m_pHeaders = NULL;
    // these header lines will overwrite/add to cURL defaults
    m_pHeaders = curl_slist_append(m_pHeaders, "Content-Type: text/xml;charset=UTF-8");//default is formurl-encoded with cURL-POST, that's bad for us
	m_pHeaders = curl_slist_append(m_pHeaders, "SOAPAction: \"http://tempuri.org/DownloadFile\"");
    m_pHeaders = curl_slist_append(m_pHeaders, "Expect:") ;

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, m_pHeaders);
	curl_easy_setopt(curl_handle, CURLOPT_URL, "http://zero-k.info/ContentService.asmx" );
	//curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, wxcurl_stream_write);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&rheader);
    curl_easy_setopt(curl_handle, CURLOPT_POST, TRUE);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data.Len() );
    curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, wxcurl_stream_read);
    curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void*)&req);

	const CURLcode ret = curl_easy_perform(curl_handle);
	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	m_buffers[index] = response.GetString();

	PlasmaResourceInfo info;
	if( ret != CURLE_OK ) {
		wxLogError( _T("dl curl error:\n%s"), m_buffers[index].c_str() );
		info.m_type = PlasmaResourceInfo::unknown;
		return info;
	}

	try {
		info = ParseResourceInfoData( index );
		return info;
	}
	catch (assert_exception& a){
		wxLogError( WX_STRINGC( a.what() ) );
	}
	catch ( ... ) {}
	info.m_type = PlasmaResourceInfo::unknown;
	return info;

}

PlasmaResourceInfo PlasmaInterface::ParseResourceInfoData( const int buffer_index )
{
    PlasmaResourceInfo info;
    wxString wxbuf = m_buffers[buffer_index];

    wxString t_begin = _T("<soap:Envelope");
    wxString t_end = _T("</soap:Envelope>");
    wxString xml_section = wxbuf.Mid( wxbuf.Find( t_begin ) );//first char after t_begin to one before t_end

    wxStringInputStream str_input( xml_section );
    wxXmlDocument xml( str_input );
	ASSERT_EXCEPTION( xml.GetRoot(), _T("Plasma: XMLparser: no root") );
    wxXmlNode *node = xml.GetRoot()->GetChildren();
	ASSERT_EXCEPTION( node , _T("Plasma: XMLparser: no first node") );
    wxString resourceType ( _T("unknown") );
    node = node->GetChildren();
	ASSERT_EXCEPTION( node , _T("Plasma: XMLparser: no node") );
    while ( node ) {
        wxString node_name = node->GetName();
        if ( node_name == _T("DownloadFileResponse") ) {
            wxXmlNode* downloadFileResult = node->GetChildren();
			ASSERT_EXCEPTION( downloadFileResult, _T("Plasma: XMLparser: no result section") );
            wxString result = downloadFileResult->GetNodeContent();
            //check result
            wxXmlNode* links = downloadFileResult->GetNext();
			ASSERT_EXCEPTION( links, _T("Plasma: XMLparser: no webseed section") );
            wxXmlNode* url = links->GetChildren();
            while ( url ) {
				wxString seed_url = url->GetNodeContent();
				seed_url.Replace(_T(" "),_T("%20"));
				info.m_webseeds.Add( seed_url );
                url = url->GetNext();
            }
            wxXmlNode* next = links->GetNext();
            while ( next ) {
                wxString next_name = next->GetName();
                if ( next_name == _T("torrentFileName") ) {
                    info.m_torrent_filename = next->GetNodeContent();
                }
                else if ( next_name == _T("dependencies") ) {
                    wxXmlNode* deps = next->GetChildren();
                    while ( deps ) {
                        info.m_dependencies.Add( deps->GetNodeContent() );
                        deps = deps->GetNext();
                    }
                }
                else if ( next_name == _T("resourceType") ) {
                    resourceType = next->GetNodeContent();
                    if ( resourceType == _T("Mod") )
                        info.m_type = PlasmaResourceInfo::mod;
                    else if ( resourceType == _T("Map") )
                        info.m_type = PlasmaResourceInfo::map;
                    else
						info.m_type = PlasmaResourceInfo::unknown;
                }
                next = next->GetNext();
            }
            break;
        } // end section <DownloadFileResponse/>
        node = node->GetNext();
    }
    wxString seeds;
    for ( size_t i = 0; i < info.m_webseeds.Count(); ++i )
        seeds += info.m_webseeds[i] + _T("\n");

    return info;
}

/** @brief downloadFile
  *
  * @todo: document this function
  */
void PlasmaInterface::downloadFile(const wxString& host, const wxString& remote_path, const wxString& local_dest) const
{
    wxHTTP FileDownloading;
    /// normal timeout is 10 minutes.. set to 10 secs.
    FileDownloading.SetTimeout(10);
    FileDownloading.Connect( host, 80);

    wxInputStream* httpstream = FileDownloading.GetInputStream( _T("/") + remote_path );
	//wxLogDebug( _T("downloading file %s/%s"),host,remote_path );

    if ( httpstream )
    {
          wxFileOutputStream outs( local_dest );
          httpstream->Read(outs);
          outs.Close();
          delete httpstream;
          httpstream = 0;
          //download success
    }
    else {
        throw std::exception();
    }
}

/** @brief DownloadTorrentFile
  *
  * @todo: document this function
  */
bool PlasmaInterface::DownloadTorrentFile( PlasmaResourceInfo& info, const wxString& destination_directory) const
{
    wxString dl_target = destination_directory + wxFileName::GetPathSeparator() + info.m_torrent_filename;
    try {
		downloadFile( m_host, _T("Resources/") + info.m_torrent_filename, dl_target );
        info.m_local_torrent_filepath = dl_target;
    }
    catch ( std::exception& e ) {
        wxLogError( _T("downloading file failed: :") + dl_target );
        return false;
    }
    return true;
}

void PlasmaInterface::InitResourceList()
{
    m_resource_list = PlasmaResourceInfo::VectorFromFile( _T("plasmaresourcelist.sl") );
}

//void PlasmaInterface::FetchResourceList()
void FetchResourceListWorkItem::Run()
{
    wxString data = s_soap_querytemplate_resourcelist;

	wxStringInputStream req ( data );
	wxStringOutputStream response;
	wxStringOutputStream rheader;
	CURL *curl_handle;
	curl_handle = curl_easy_init();
	struct curl_slist* m_pHeaders = NULL;
	// these header lines will overwrite/add to cURL defaults
	m_pHeaders = curl_slist_append(m_pHeaders, "Content-Type: text/xml;charset=UTF-8");//default is formurl-encoded with cURL-POST, that's bad for us
	m_pHeaders = curl_slist_append(m_pHeaders, "SOAPAction: \"http://planet-wars.eu/PlasmaServer/GetResourceList\"");
	m_pHeaders = curl_slist_append(m_pHeaders, "Expect:") ;

	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, m_pHeaders);
	curl_easy_setopt(curl_handle, CURLOPT_URL, "http://zero-k.info/ContentService.asmx" );
	//curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, wxcurl_stream_write);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, (void *)&rheader);
	curl_easy_setopt(curl_handle, CURLOPT_POST, TRUE);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, data.Len() );
	curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, wxcurl_stream_read);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, (void*)&req);

	CURLcode ret = curl_easy_perform(curl_handle);
	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if ( ret != CURLE_OK ) {
		GetGlobalEventSender( GlobalEvents::PlasmaResourceListFailedDownload ).SendEvent( 0 );
	}
	else {
		m_interface->ParseResourceListData( response.GetString() );
		GetGlobalEventSender( GlobalEvents::PlasmaResourceListParsed ).SendEvent( 0 );
	}

}
void PlasmaInterface::ParseResourceListData( const wxString& buffer )
{
	//MUTEX !!
	wxString wxbuf = buffer;

    wxString t_begin = _T("<soap:Envelope");
    wxString t_end = _T("</soap:Envelope>");
    wxString xml_section = wxbuf.Mid( wxbuf.Find( t_begin ) );//first char after t_begin to one before t_end

    wxStringInputStream str_input( xml_section );
    wxXmlDocument xml( str_input );
    assert( xml.GetRoot() );
    wxXmlNode *node = xml.GetRoot()->GetChildren();
    assert( node );

    m_resource_list.clear();

    wxString resourceType ( _T("unknown") );
    node = node->GetChildren();
    assert( node );
    while ( node ) {
        wxString node_name = node->GetName();
        if ( node_name == _T("GetResourceListResponse") ) {
            wxXmlNode* resourceListResult = node->GetChildren();
            assert( resourceListResult );
            while ( resourceListResult ) {
                wxXmlNode* resourceData = resourceListResult->GetChildren();

                while ( resourceData ) {
                    wxXmlNode* resourceDataContent = resourceData->GetChildren();
                    PlasmaResourceInfo info;
                    while ( resourceDataContent ) {
                        wxString rc_node_name = resourceDataContent->GetName();
                        if ( rc_node_name == _T("Dependencies") ){
                            //! TODO
                        }
                        else if ( rc_node_name == _T("InternalName") ){
                            info.m_name = resourceDataContent->GetNodeContent();
                        }
                        else if ( rc_node_name == _T("ResourceType") ){
                            resourceType = resourceDataContent->GetNodeContent();
                            if ( resourceType == _T("Mod") )
                                info.m_type = PlasmaResourceInfo::mod;
                            else if ( resourceType == _T("Map") )
                                info.m_type = PlasmaResourceInfo::map;
                            else
								info.m_type = PlasmaResourceInfo::unknown;
                        }
                        else if ( rc_node_name == _T("SpringHashes") ){
                            //! TODO
                        }
                        resourceDataContent = resourceDataContent->GetNext();
                    }
                    m_resource_list.push_back( info );
                    resourceData = resourceData->GetNext();
                }
                resourceListResult = resourceListResult->GetNext();
            }
        } // end section <GetResourceListResponse/>
        node = node->GetNext();
    }
	PlasmaResourceInfo::VectorToFile( m_resource_list, _T("plasmaresourcelist.sl") );
}

void FetchResourceListWorkItem ::SetInterface( PlasmaInterface* i )
{
    m_interface = i;
}

void PlasmaInterface::FetchResourceList()
{
	FetchResourceListWorkItem* item = new FetchResourceListWorkItem( );
	item->SetInterface( this );
	m_worker_thread.DoWork( item, 0 );
}

#endif //NO_TORRENT_SYSTEM
