// RtcDemoDlg.cpp : implementation file
//
#include "stdafx.h"
#include "RtcDemo.h"
#include "RtcDemoDlg.h"
#include <future>
#include <fstream>
#include "afxdialogex.h"
#include "charactor_convert.h"
#include "qn_rtc_engine.h"
#include "curl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#pragma comment(lib, "QNRtcStreamingD.lib")
#pragma comment(lib, "libcurld.lib")
#else
#pragma comment(lib, "QNRtcStreaming.lib")
#pragma comment(lib, "libcurl.lib")
#endif // _DEBUG
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "Version.lib")

#define CUSTOM_RESOURCE_ID         10000
#define CALLBACK_UI_TIMER_ID       1
#define UPDATE_TIME_TIMER_ID       2     // ��������ʱ�䶨ʱ��
#define DISPLAY_NAME_HEIGHT        18    // pix

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

                                                        // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CRtcDemoDlg dialog

CRtcDemoDlg::CRtcDemoDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_RTCDEMO_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRtcDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PLAYER, _user_list_ctrl);
}

BEGIN_MESSAGE_MAP(CRtcDemoDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CRtcDemoDlg::OnBnClickedButtonJoin)
    ON_BN_CLICKED(IDC_BUTTON_PUBLISH, &CRtcDemoDlg::OnBnClickedButtonPublish)
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_CBN_SELCHANGE(IDC_COMBO_CAMERA, &CRtcDemoDlg::OnCbnSelchangeComboCamera)
    ON_CBN_SELCHANGE(IDC_COMBO_MICROPHONE, &CRtcDemoDlg::OnCbnSelchangeComboMicrophone)
    ON_BN_CLICKED(IDC_BUTTON_PREVIEW_VIDEO, &CRtcDemoDlg::OnBnClickedButtonPreview)
    ON_BN_CLICKED(IDC_BTN_KICKOUT, &CRtcDemoDlg::OnBnClickedBtnKickout)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_CHECK_MUTE_AUDIO, &CRtcDemoDlg::OnBnClickedCheckMuteAudio)
    ON_BN_CLICKED(IDC_CHECK_MUTE_VIDEO, &CRtcDemoDlg::OnBnClickedCheckMuteVideo)
    ON_CBN_SELCHANGE(IDC_COMBO_PLAYOUT, &CRtcDemoDlg::OnCbnSelchangeComboPlayout)
    ON_WM_TIMER()
END_MESSAGE_MAP()

// CRtcDemoDlg message handlers

static string GetAppVersion()
{
    DWORD dwInfoSize  = 0;
    char exePath[MAX_PATH];
    char ver_buf[128] = { 0 };
    int ver_buf_len   = 0;
    memset(exePath, 0, sizeof(exePath));

    // �õ����������·��
    GetModuleFileNameA(NULL, exePath, sizeof(exePath) / sizeof(char));

    // �ж��Ƿ��ܻ�ȡ�汾��
    dwInfoSize = GetFileVersionInfoSizeA(exePath, NULL);

    if (dwInfoSize == 0) {
        return "";
    } else {
        BYTE* pData = new BYTE[dwInfoSize];
        // ��ȡ�汾��Ϣ
        if (!GetFileVersionInfoA(exePath, NULL, dwInfoSize, pData)) {
            return "";
        } else {
            // ��ѯ�汾��Ϣ�еľ����ֵ
            LPVOID lpBuffer;
            UINT uLength;
            if (!::VerQueryValue((LPCVOID)pData, _T("\\"), &lpBuffer, &uLength)) {
            } else {
                DWORD dwVerMS;
                DWORD dwVerLS;
                dwVerMS = ((VS_FIXEDFILEINFO*)lpBuffer)->dwProductVersionMS;
                dwVerLS = ((VS_FIXEDFILEINFO*)lpBuffer)->dwProductVersionLS;
                ver_buf_len = snprintf(ver_buf,
                    sizeof(ver_buf),
                    "Version : %d.%d.%d.%d",
                    (dwVerMS >> 16),
                    (dwVerMS & 0xFFFF),
                    (dwVerLS >> 16),
                    (dwVerLS & 0xFFFF));
            }
        }
        delete pData;
    }
    return string(ver_buf, ver_buf_len);
}

BOOL CRtcDemoDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);     // Set big icon
    SetIcon(m_hIcon, FALSE);    // Set small icon

    // Init Status Bar
    InitStatusBar();

    // ��ȡ���ü�¼������ʼ�� UI
    ReadConfigFile();

    qiniu::QNRTCEngine::Init();

    // ������־������ļ���������û����־���
    qiniu::QNRTCEngine::SetLogParams(qiniu::LOG_INFO, "rtc-log", "rtc.log");

    _rtc_room_interface = QNRTCRoom::ObtainRoomInterface();
    if (!_rtc_room_interface) {
        return FALSE;
    }
    _rtc_room_interface->SetRoomListener(this);

    // ��Ƶ���ܽӿ�
    _rtc_video_interface = _rtc_room_interface->ObtainVideoInterface();
    if (!_rtc_video_interface) {
        return FALSE;
    }
    _rtc_video_interface->SetVideoListener(this);

    // ��Ƶ���ܽӿ�
    _rtc_audio_interface = _rtc_room_interface->ObtainAudioInterface();
    if (!_rtc_audio_interface) {
        return FALSE;
    }
    _rtc_audio_interface->SetAudioListener(this);

    // �������������ص����ʱ�䣬��λ����
    _rtc_room_interface->EnableStatisticCallback(5);

    // ��ʼ���û��б��ؼ�
    _user_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    _user_list_ctrl.InsertColumn(0, _T("�û� ID"), LVCFMT_LEFT, 100, 0);    //������
    _user_list_ctrl.InsertColumn(1, _T("�û�������״̬"), LVCFMT_LEFT, 350, 1);
    ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->SetCheck(1);
    ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->SetCheck(1);

    // ��ʼ����Ƶ�ɼ��豸 combobox
    for (int i(0); i < _rtc_video_interface->GetCameraCount(); ++i) {
        CameraDeviceInfo ci = _rtc_video_interface->GetCameraInfo(i);
        _camera_dev_map[ci.device_id] = ci;
        ((CComboBox *)GetDlgItem(IDC_COMBO_CAMERA))->InsertString(-1, utf2unicode(ci.device_name).c_str());
    }
    ((CComboBox *)GetDlgItem(IDC_COMBO_CAMERA))->SetCurSel(0);

    // ��ʼ����Ƶ�ɼ��豸�б�
    for (int i(0); i < _rtc_audio_interface->GetAudioDeviceCount(AudioDeviceInfo::adt_record); ++i) {
        AudioDeviceInfo audio_info;
        if (_rtc_audio_interface->GetAudioDeviceInfo(AudioDeviceInfo::adt_record, i, audio_info) == 0) {
            ((CComboBox *)GetDlgItem(IDC_COMBO_MICROPHONE))->InsertString(
                -1,
                utf2unicode(audio_info.device_name).c_str()
            );
        }
    }
    ((CComboBox *)GetDlgItem(IDC_COMBO_MICROPHONE))->SetCurSel(0);

    // ��ʼ����Ƶ�����豸�б�
    for (int i(0); i < _rtc_audio_interface->GetAudioDeviceCount(AudioDeviceInfo::adt_record); ++i) {
        AudioDeviceInfo audio_info;
        if (_rtc_audio_interface->GetAudioDeviceInfo(AudioDeviceInfo::adt_playout, i, audio_info) == 0) {
            ((CComboBox *)GetDlgItem(IDC_COMBO_PLAYOUT))->InsertString(
                -1,
                utf2unicode(audio_info.device_name).c_str()
            );
        }
    }
    ((CComboBox *)GetDlgItem(IDC_COMBO_PLAYOUT))->SetCurSel(0);

    // ��ʼ����������������
    ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD))->SetRange(0, 255);
    ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD))->SetPos(255);
    ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAYOUT))->SetRange(0, 255);
    ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAYOUT))->SetPos(255);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRtcDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRtcDemoDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRtcDemoDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CRtcDemoDlg::OnStateChanged(RoomState status_)
{
    if (status_ == qiniu::rs_reconnecting) {
        // ����Ͽ���������... �û���ʲô��������SDK �ڲ��᲻�ϵĳ�������
        _wndStatusBar.SetText(_T("����Ͽ��������С�����"), 1, 0);
    }
}

void CRtcDemoDlg::OnJoinResult(int error_code_, const std::string& error_str_,
    const UserDataInfoVec& user_data_vec_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([=]() {
        // ȡ��ԭ�����еĶ���, ���ͷ���Դ
        for (auto&& itor : _user_stream_map) {
            if (itor.second.is_subscribed) {
                _rtc_room_interface->UnSubscribe(itor.first);
            }
        }
        _user_stream_map.clear();

        // �������
        GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
        if (error_code_ != 0 || user_data_vec_.empty()) {
            CString msg_str;
            msg_str.Format(_T("��¼����ʧ�ܣ������룺%d��������Ϣ��%s"), error_code_, utf2unicode(error_str_).c_str());
            _wndStatusBar.SetText(msg_str, 1, 0);
            std::thread([&, msg_str]() {
                MessageBox(msg_str, _T("��¼ʧ�ܣ�"));
            }).detach();
            GetDlgItem(IDC_BUTTON_LOGIN)->SetWindowTextW(_T("��¼"));
            return;
        }
        if (user_data_vec_.empty()) {
            _wndStatusBar.SetText(_T("�������ظ��쳣��û���κ������Ϣ!"), 1, 0);
            std::thread([&]() {
                MessageBox(_T("�������ظ��쳣��û���κ������Ϣ!"), _T("��¼ʧ�ܣ�"));
            }).detach();
            GetDlgItem(IDC_BUTTON_LOGIN)->SetWindowTextW(_T("��¼"));
            return;
        }
        _wndStatusBar.SetText(_T("��¼�ɹ�!"), 1, 0);
        SetDlgItemText(IDC_BUTTON_LOGIN, _T("�뿪"));
        GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
        CString local_user_id;
        GetDlgItemText(IDC_EDIT_PLAYER_ID, local_user_id);

        // ���ؼ�¼�û���Ϣ
        for each (UserDataInfo itor in user_data_vec_)
        {
            UserStreamInfo info;
            info.user_id = itor.user_id;
            _user_stream_map[itor.user_id] = info;
        }

        _user_list_ctrl.DeleteAllItems();
        for each (UserDataInfo itor in user_data_vec_)
        {
            if (local_user_id.CompareNoCase(utf2unicode(itor.user_id).c_str()) == 0) {
                // not show self
                continue;
            }
            _user_list_ctrl.InsertItem(0, utf2unicode(itor.user_id).c_str());
            _user_list_ctrl.SetItemText(0, 0, utf2unicode(itor.user_id).c_str());

            CString publish_state_str;
            publish_state_str.Format(_T("Audio:%s,Video:%s,Mute-Audio:%s,Mute-Video:%s"),
                itor.audio_published ? _T("true") : _T("false"),
                itor.video_published ? _T("true") : _T("false"),
                itor.audio_mute ? _T("true") : _T("false"),
                itor.video_mute ? _T("true") : _T("false")
            );

            _user_list_ctrl.SetItemText(0, 1, publish_state_str);
            
            // �Զ���������Զ���û�������
            if (itor.video_published || itor.audio_published) {
                OnRemotePublish(itor.user_id, itor.video_published, itor.audio_published);
            }
        }
        // ��¼��ʼ�����ϵͳʱ�䣬��������ʱ��
        _start_time = chrono::system_clock::now();
        SetTimer(UPDATE_TIME_TIMER_ID, 1000, nullptr);
    });
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnLeave(int error_code_,
    const std::string& error_str_, const std::string& kicked_user_id_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, error_code_, error_str_, kicked_user_id_]() {
        if (error_code_ == Err_Kickout_Of_Room) {
            // �����¿�һ���߳̽��е���
            std::thread([&] {
                char buf[1024] = { 0 };
                snprintf(buf,
                    sizeof(buf),
                    "���� %s �߳��˷��䣡\r\n �����Ҫ���µ�¼���䣬�������ֶ���¼��",
                    kicked_user_id_.c_str());
                MessageBox(utf2unicode(buf).c_str());
            }).detach();
        }
        _wndStatusBar.SetText(_T("�����뿪���䣡"), 1, 0);
        SetDlgItemText(IDC_BUTTON_LOGIN, _T("��¼"));
        _user_list_ctrl.DeleteAllItems();
        SetDlgItemText(IDC_BUTTON_PREVIEW_VIDEO, _T("Ԥ��"));
        SetDlgItemText(IDC_BUTTON_PUBLISH, _T("����"));
        GetDlgItem(IDC_STATIC_VIDEO_PREVIEW)->Invalidate();
        _user_stream_map.clear();
        _rtc_room_interface->LeaveRoom();
    });
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnRemoteUserJoin(const std::string& user_id_, const std::string& user_data_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, user_id_, user_data_]() {
        auto itor = _user_stream_map.find(user_id_);
        if (itor != _user_stream_map.end()) {
            CString str;
            for (int i = 0; i < _user_list_ctrl.GetItemCount(); i++) {
                str = _user_list_ctrl.GetItemText(i, 0);
                if (str.CompareNoCase(utf2unicode(user_id_).c_str()) == 0) {
                    _user_list_ctrl.DeleteItem(i);
                    break;
                }
            }
            _user_stream_map.erase(itor);
        }
        _user_list_ctrl.InsertItem(0, utf2unicode(user_id_).c_str());
        _user_list_ctrl.SetItemText(0, 1, _T("Audio:false,Video:false,Mute-Audio:false,Mute-Video:false"));

        CString str;
        str.Format(_T("%s �����˷��䣡"), utf2unicode(user_id_).c_str());
        _wndStatusBar.SetText(
            str, 1, 0);

        UserStreamInfo info;
        info.user_id = user_id_;
        _user_stream_map[user_id_] = info;
    });
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnRemoteUserLeave(const std::string& user_id_, int error_code)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, user_id_, error_code]() {
        auto itor = _user_stream_map.find(user_id_);
        if (itor != _user_stream_map.end()) {
            CString str;
            for (int i = 0; i < _user_list_ctrl.GetItemCount(); i++) {
                str = _user_list_ctrl.GetItemText(i, 0);
                if (str.CompareNoCase(utf2unicode(itor->first).c_str()) == 0) {
                    _user_list_ctrl.DeleteItem(i);
                    break;
                }
            }
            itor->second.render_wnd_ptr.reset();
            _user_stream_map.erase(itor);

            str.Format(_T("%s �뿪�˷��䣡"), utf2unicode(user_id_).c_str());
            _wndStatusBar.SetText(
                str, 1, 0);
        }
    });
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnRemotePublish(const std::string& user_id_, bool has_audio_, bool has_video_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, user_id_, has_audio_, has_video_]() {
        auto itor = _user_stream_map.find(user_id_);
        if (itor == _user_stream_map.end()) {
            return;
        }
        itor->second.audio_published = has_audio_;
        itor->second.video_published = has_video_;
        itor->second.is_subscribed = false;
        CString str, item_str;
        for (int i = 0; i < _user_list_ctrl.GetItemCount(); i++) {
            str = _user_list_ctrl.GetItemText(i, 0);
            if (str.CompareNoCase(utf2unicode(itor->first).c_str()) == 0) {
                item_str.Format(_T("Audio:%s,Video:%s,Mute-Audio:%s,Mute-Video:%s"),
                    itor->second.audio_published ? _T("true") : _T("false"),
                    itor->second.video_published ? _T("true") : _T("false"),
                    itor->second.audio_mute ? _T("true") : _T("false"),
                    itor->second.video_mute ? _T("true") : _T("false")
                );                    
                _user_list_ctrl.SetItemText(i, 1, item_str.GetBuffer());
                break;
            }
        }
        itor->second.display_name_ptr.reset(new CStatic, [](CWnd* ptr_) {
            if (ptr_) {
                ptr_->Invalidate();
                ptr_->DestroyWindow();
                delete ptr_;
            }
        });
        itor->second.render_wnd_ptr.reset(new CStatic, [](CWnd* ptr_) {
            if (ptr_) {
                ptr_->Invalidate();
                ptr_->DestroyWindow();
                delete ptr_;
            }
        });
        CRect rc = GetRenderWndPos();
        itor->second.render_wnd_ptr->Create(
            _T(""),
            WS_CHILD | WS_VISIBLE | WS_DISABLED,
            rc,
            GetDlgItem(IDC_STATIC_PLAY),
            CUSTOM_RESOURCE_ID + ++_resource_id);
        itor->second.display_name_ptr->Create(
            utf2unicode(itor->first).c_str(),
            WS_CHILD | WS_VISIBLE | WS_DISABLED || SS_CENTER,
            CRect(0, 0, rc.right, DISPLAY_NAME_HEIGHT),
            GetDlgItem(IDC_STATIC_PLAY),
            CUSTOM_RESOURCE_ID + ++_resource_id);
        
        itor->second.display_name_ptr->ShowWindow(SW_SHOWNORMAL);
        itor->second.render_wnd_ptr->ShowWindow(SW_SHOWNORMAL);
        _rtc_room_interface->Subscribe(itor->first, itor->second.render_wnd_ptr->m_hWnd);
        if (_contain_admin_flag) {
            AdjustMergeStreamPosition();
        }
        str.Format(_T("%s ������ý������"), utf2unicode(user_id_).c_str());
        _wndStatusBar.SetText(
            str, 1, 0);
    }
    );
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnRemoteUnPublish(const std::string& user_id_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, user_id_]() {
        auto itor = _user_stream_map.find(user_id_);
        if (itor == _user_stream_map.end()) {
            return;
        }
        itor->second.audio_published = false;
        itor->second.video_published = false;
        itor->second.is_subscribed   = false;
        CString str;
        for (int i = 0; i < _user_list_ctrl.GetItemCount(); i++) {
            str = _user_list_ctrl.GetItemText(i, 0);
            if (str.CompareNoCase(utf2unicode(itor->first).c_str()) == 0) {
                _user_list_ctrl.SetItemText(i, 1, _T("Audio:false,Video:false,Mute-Audio:false,Mute-Video:false"));
                break;
            }
        }
        itor->second.render_wnd_ptr.reset();
        itor->second.display_name_ptr.reset();

        if (_contain_admin_flag) {
            AdjustMergeStreamPosition();
        }
        str.Format(_T("%s ȡ��������ý������"), utf2unicode(user_id_).c_str());
        _wndStatusBar.SetText(
            str, 1, 0);
    }
    );
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnLocalPublishResult(int error_code_, const std::string& error_str_)
{
    if (0 == error_code_) {
        lock_guard<recursive_mutex> lock_(_mutex);
        _call_function_vec.emplace_back([&, error_code_, error_str_]() {
            _publish_flag = true;
            SetDlgItemText(IDC_STATIC_PUBLISH_STREAM_ID, _T("publish success"));
            SetDlgItemText(IDC_BUTTON_PUBLISH, _T("ȡ������"));
            SetDlgItemText(IDC_BUTTON_PREVIEW_VIDEO, _T("Ԥ��"));
            ((CButton *)GetDlgItem(IDC_CHECK_MUTE_AUDIO))->SetCheck(0);
            ((CButton *)GetDlgItem(IDC_CHECK_MUTE_VIDEO))->SetCheck(0);
            if (_contain_admin_flag) {
                _rtc_room_interface->SetMergeStreamLayout(
                    unicode2utf(_user_id.GetBuffer()), 0, 0, 0, Canvas_Width, Canvas_Height, true);
            }
            _wndStatusBar.SetText(_T("�����������ɹ���"), 1, 0);
        });
        SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
    } else {
        MessageBox(_T("����ʧ�ܣ���ȷ������Ƶ�ɼ��豸���������򿪣� "), _T("����ʧ�ܣ� "));
    }
}

void CRtcDemoDlg::OnSubscribeResult(const std::string& user_id_,
    int error_code_, const std::string& error_str_)
{
    if (0 != error_code_) {
        CString str;
        str.Format(_T("�����û�:%s ������ʧ��, error code��%d�� erro string��%s"),
            utf2unicode(user_id_).c_str(), error_code_, utf2unicode(error_str_).c_str());

        _wndStatusBar.SetText(str, 1, 0);

        std::thread([=]() {
            MessageBox(str);
        }).detach();

        return;
    }
    lock_guard<recursive_mutex> lock_(_mutex);
    if (_user_stream_map.empty()) {
        //û���κ��û���ý������¼
        return;
    }
    auto itor = _user_stream_map.find(user_id_);
    if (itor != _user_stream_map.end()) {
        if (itor->first.compare(user_id_) == 0) {
            itor->second.is_subscribed = true;
        }
    }
    CString str;
    str.Format(_T("���� %s �������ɹ���"), utf2unicode(user_id_).c_str());
    _wndStatusBar.SetText(str, 1, 0);
}

void CRtcDemoDlg::OnKickoutResult(const std::string& user_id_,
    int error_code_, const std::string& error_str_)
{
    char buf[1024] = { 0 };
    if (0 == error_code_) {
        snprintf(buf, 1024, "�߳��û���%s �ɹ��� ", user_id_.c_str());
    } else {
        snprintf(buf, 1024, "�߳��û���%s ʧ�ܣ� �����룺%d�� %s",
            user_id_.c_str(), error_code_, error_str_.c_str());
    }
    _wndStatusBar.SetText(utf2unicode(buf).c_str(), 1, 0);
    MessageBox(utf2unicode(buf).c_str(), _T("�߳��û��� "));
}

void CRtcDemoDlg::OnRemoteStreamMute(const std::string& user_id_,
    bool mute_audio_, bool mute_video_)
{
    lock_guard<recursive_mutex> lock_(_mutex);
    _call_function_vec.emplace_back([&, user_id_, mute_audio_, mute_video_]() {

        auto itor = _user_stream_map.find(user_id_);
        if (itor == _user_stream_map.end()) {
            return;
        }
        itor->second.audio_mute = mute_audio_;
        itor->second.video_mute = mute_video_;
        CString str;
        for (int i = 0; i < _user_list_ctrl.GetItemCount(); i++) {
            str = _user_list_ctrl.GetItemText(i, 0);
            if (str.CompareNoCase(utf2unicode(user_id_).c_str()) == 0) {
                wchar_t buf[128] = { 0 };
                wsprintf(buf, 
                    _T("Audio:%s,Video:%s,Mute-Audio:%s,Mute-Video:%s"),
                    itor->second.audio_published?_T("true"):_T("false"),
                    itor->second.video_published ? _T("true") : _T("false"),
                    itor->second.audio_mute ? _T("true") : _T("false"),
                    itor->second.video_mute ? _T("true") : _T("false")
                );
                _user_list_ctrl.SetItemText(i, 1, buf);
                break;
            }
        }
    });
    SetTimer(CALLBACK_UI_TIMER_ID, 1, nullptr);
}

void CRtcDemoDlg::OnError(int error_code_, const std::string& error_str_)
{
    std::thread([=]() {
        char buf[1024] = { 0 };
        snprintf(buf, sizeof(buf), "Error code:%d, error string:%s", error_code_, error_str_.c_str());
        _wndStatusBar.SetText(utf2unicode(buf).c_str(), 1, 0);
        MessageBox(utf2unicode(buf).c_str());
    }
    ).detach();
}

void CRtcDemoDlg::OnStatisticsUpdated(const StatisticsReport& statistics_)
{
    char dest_buf[1024] = { 0 };
    snprintf(dest_buf,
        sizeof(dest_buf),
        "user:%s statistics: audio bitrate:%d, audio packet lost rate:%0.3f, video width:%d,"
        "video height:%d, video frame rate:%d, video bitrate:%d, video packet lost rate:%0.3f",
        statistics_.user_id.c_str(),
        statistics_.audio_bitrate,
        statistics_.audio_packet_lost_rate,
        statistics_.video_width,
        statistics_.video_height,
        statistics_.video_frame_rate,
        statistics_.video_bitrate,
        statistics_.video_packet_lost_rate
    );
    TRACE(utf2unicode(dest_buf).c_str());
}

void CRtcDemoDlg::OnAudioPCMFrame(const void* audio_data_,
    int bits_per_sample_, int sample_rate_, size_t number_of_channels_,
    size_t number_of_frames_, const std::string& user_id_)
{

}

void CRtcDemoDlg::OnAudioDeviceStateChanged(
    AudioDeviceState new_device_state_, const std::string& device_guid_)
{
    if (new_device_state_ != ads_active) {
        char msg_buf[1024] = { 0 };
        snprintf(msg_buf,
            sizeof(msg_buf),
            "��Ƶ�豸��%s ���γ���ʧЧ�ˣ����ܻ�Ӱ��������������",
            device_guid_.c_str());

        string msg_str(msg_buf);
        _wndStatusBar.SetText(utf2unicode(msg_str).c_str(), 1, 0);
        std::thread([=]() {
            MessageBox(utf2unicode(msg_str).c_str());
        }).detach();
    }
}

void CRtcDemoDlg::OnVideoFrame(const unsigned char* raw_data_,
    int data_len_, qiniu::VideoCaptureType video_type_, const std::string& user_id_)
{

}

void CRtcDemoDlg::OnVideoDeviceStateChanged(
    VideoDeviceState new_device_state_, const std::string& device_id_)
{
    if (new_device_state_ != vds_active) {
        char msg_buf[1024] = { 0 };
        snprintf(msg_buf,
            sizeof(msg_buf),
            "��Ƶ�豸��%s ���γ�����������������У��������豸�����·�����",
            device_id_.c_str());

        string msg_str(msg_buf);
        _wndStatusBar.SetText(utf2unicode(msg_str).c_str(), 1, 0);
        std::thread([=]() {
            MessageBox(utf2unicode(msg_str).c_str());
        }).detach();
    } else if (new_device_state_ != vds_lost) {
        //�豸����
        char msg_buf[1024] = { 0 };
        snprintf(msg_buf,
            sizeof(msg_buf),
            "��Ƶ�豸��%s �Ѳ��룬��������������У������·�����",
            device_id_.c_str());

        string msg_str(msg_buf);
        _wndStatusBar.SetText(utf2unicode(msg_str).c_str(), 1, 0);
        std::thread([=]() {
            MessageBox(utf2unicode(msg_str).c_str());
        }).detach();
    }
}

void CRtcDemoDlg::OnBnClickedButtonJoin()
{
    KillTimer(UPDATE_TIME_TIMER_ID);

    CString btn_str;
    GetDlgItemText(IDC_BUTTON_LOGIN, btn_str);
    if (btn_str.CompareNoCase(_T("��¼")) == 0) {
        GetDlgItemText(IDC_EDIT_ROOM_ID, _room_name);
        GetDlgItemText(IDC_EDIT_PLAYER_ID, _user_id);
        if (_room_name.IsEmpty() || _user_id.IsEmpty()) {
            MessageBox(_T("Room ID and Player ID can't be NULL!"));
            return;
        }
        GetDlgItem(IDC_BUTTON_LOGIN)->SetWindowText(_T("��¼��"));
        GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(FALSE);

        if (_join_room_thread.joinable()) {
            _join_room_thread.join();
        }
        _join_room_thread = std::thread([this]() {
            //�� AppServer ��ȡ token 
            _room_token.clear();
            int ret = GetRoomToken(unicode2utf(_room_name.GetBuffer()),
                unicode2utf(_user_id.GetBuffer()), _room_token);
            if (ret != 0) {
                _wndStatusBar.SetText(_T("��ȡ���� token ʧ�ܣ��������������Ƿ�������"), 1, 0);
                MessageBox(_T("��ȡ���� token ʧ�ܣ��������������Ƿ�������"));
                GetDlgItem(IDC_BUTTON_LOGIN)->SetWindowText(_T("��¼"));
                GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
                return;
            }
            _wndStatusBar.SetText(_T("��ȡ���� token �ɹ���"), 1, 0);
            _rtc_room_interface->JoinRoom(_room_token);

            WriteConfigFile();
        });
    } else {
        // LeaveRoom
        if (_contain_admin_flag) {
            _rtc_room_interface->StopMergeStream();
        }
        _rtc_room_interface->LeaveRoom();
        SetDlgItemText(IDC_BUTTON_LOGIN, _T("��¼"));
        SetDlgItemText(IDC_BUTTON_PUBLISH, _T("����"));
        GetDlgItem(IDC_BUTTON_PUBLISH)->Invalidate();
        _user_list_ctrl.DeleteAllItems();
        _user_stream_map.clear();
        _wndStatusBar.SetText(_T("��ǰδ��¼���䣡"), 1, 0);

        Invalidate();
    }
}

void CRtcDemoDlg::OnBnClickedButtonPublish()
{
    // TODO: Add your control notification handler code here
    CString str;
    GetDlgItemText(IDC_BUTTON_PUBLISH, str);
    if (0 == str.CompareNoCase(_T("����"))) {
        CString video_dev_name;
        string video_dev_id;
        int audio_recorder_device_index(-1);
        int audio_playout_device_index(-1);
        bool enable_audio, enable_video;

        GetDlgItem(IDC_COMBO_CAMERA)->GetWindowTextW(video_dev_name);
        audio_recorder_device_index = ((CComboBox *)GetDlgItem(IDC_COMBO_MICROPHONE))->GetCurSel();
        audio_playout_device_index = ((CComboBox *)GetDlgItem(IDC_COMBO_PLAYOUT))->GetCurSel();
        enable_audio =
            (BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck()) ? true : false;
        enable_video =
            (BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck()) ? true : false;

        if (video_dev_name.IsEmpty()) {
            MessageBox(_T("����ǰû���κ���Ƶ�豸��"));
            return;
        }
        auto itor = _camera_dev_map.begin();
        while (itor != _camera_dev_map.end()) {
            if (itor->second.device_name.compare(unicode2utf(video_dev_name.GetBuffer())) == 0) {
                video_dev_id = itor->first;
                break;
            }
            ++itor;
        }
        // ��ȡ���еĳߴ�
        auto tuple_size = FindBestVideoSize(itor->second.capability_vec);

        CameraSetting camera_setting;
        camera_setting.render_hwnd = GetDlgItem(IDC_STATIC_VIDEO_PREVIEW)->m_hWnd;
        camera_setting.device_name = unicode2utf(video_dev_name.GetBuffer());
        camera_setting.device_id   = video_dev_id;
        camera_setting.width       = std::get<0>(tuple_size);
        camera_setting.height      = std::get<1>(tuple_size);
        camera_setting.max_fps     = 15;
        camera_setting.bitrate     = 500000;
        _rtc_room_interface->ObtainVideoInterface()->SetCameraParams(camera_setting);

        /* ���û����Ƶ�����豸���򲻷�����Ƶ */
        if (audio_recorder_device_index >= 0) {
            AudioDeviceSetting audio_setting;
            audio_setting.device_index = audio_recorder_device_index;
            audio_setting.device_type = qiniu::AudioDeviceSetting::wdt_DefaultDevice;
            if (0 != _rtc_audio_interface->SetRecordingDevice(audio_setting)) {
                std::thread([this] {
                    MessageBox(_T("������Ƶ�����豸ʧ�ܣ�Ӧ�ó��򽫼������󣬵����ٷ�����Ƶ����"));
                }).detach();
                audio_recorder_device_index = -1;
                _wndStatusBar.SetText(_T("������Ƶ�����豸ʧ�ܣ�Ӧ�ó��򽫼������󣬵����ٷ�����Ƶ����"), 1, 0);
            }
        } else {
            _wndStatusBar.SetText(_T("����ǰû���κ���Ƶ�����豸������������Ƶ����"), 1, 0);
        }
        if (enable_audio) {
            enable_audio = (audio_recorder_device_index < 0) ? false : true;
        }

        if (!enable_audio && !enable_video) {
            _wndStatusBar.SetText(_T("����ͬʱ������������Ƶ��"), 1, 0);
            MessageBox(_T("����ͬʱ������������Ƶ��"));
            return;
        }

        _rtc_room_interface->Publish(enable_audio, enable_video);
    } else {
        if (_contain_admin_flag) {
            _rtc_room_interface->StopMergeStream();
        }

        _rtc_room_interface->UnPublish();

        SetDlgItemText(IDC_STATIC_PUBLISH_STREAM_ID, _T(""));
        SetDlgItemText(IDC_BUTTON_PUBLISH, _T("����"));
        GetDlgItem(IDC_STATIC_VIDEO_PREVIEW)->Invalidate();
        _wndStatusBar.SetText(_T("ֹͣ������������"), 1, 0);
    }
}

void CRtcDemoDlg::OnDestroy()
{
    KillTimer(UPDATE_TIME_TIMER_ID);

    _user_stream_map.clear();
    if (_join_room_thread.joinable()) {
        _join_room_thread.join();
    }
    if (_rtc_room_interface) {
        if (_contain_admin_flag) {
            _rtc_room_interface->StopMergeStream();
        }
        _rtc_room_interface->Release();
        _rtc_room_interface = nullptr;
    }
    qiniu::QNRTCEngine::Release();

    _wndStatusBar.DestroyWindow();

    __super::OnDestroy();
}

int CRtcDemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here
    // �̶������ڴ�С 
    DWORD   dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    dwStyle &= ~(WS_SIZEBOX);
    SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);

    return 0;
}

void CRtcDemoDlg::OnCbnSelchangeComboCamera()
{
    // TODO: Add your control notification handler code here
}


void CRtcDemoDlg::OnCbnSelchangeComboMicrophone()
{
    // TODO: Add your control notification handler code here
}


void CRtcDemoDlg::OnBnClickedButtonPreview()
{
    // TODO: Add your control notification handler code here
    if (!_rtc_video_interface) {
        return;
    }
    CString str;
    GetDlgItemText(IDC_BUTTON_PREVIEW_VIDEO, str);
    if (0 == str.CompareNoCase(_T("Ԥ��"))) {
        CString cur_dev_name;
        string cur_dev_id;
        GetDlgItem(IDC_COMBO_CAMERA)->GetWindowTextW(cur_dev_name);
        if (cur_dev_name.IsEmpty()) {
            MessageBox(_T("����ǰû���κ���Ƶ�豸��"));
            return;
        }
        auto itor = _camera_dev_map.begin();
        while (itor != _camera_dev_map.end()) {
            if (itor->second.device_name.compare(unicode2utf(cur_dev_name.GetBuffer())) == 0) {
                cur_dev_id = itor->first;
                break;
            }
            ++itor;
        }
        auto tuple_size = FindBestVideoSize(itor->second.capability_vec);

        CameraSetting camera_setting;
        camera_setting.render_hwnd = GetDlgItem(IDC_STATIC_VIDEO_PREVIEW)->m_hWnd;
        camera_setting.device_name = unicode2utf(cur_dev_name.GetBuffer());
        camera_setting.device_id   = cur_dev_id;
        camera_setting.width       = std::get<0>(tuple_size);
        camera_setting.height      = std::get<1>(tuple_size);
        camera_setting.max_fps     = 15;
        camera_setting.bitrate     = 500000;

        if (0 == _rtc_video_interface->PreviewCamera(camera_setting)) {
            _wndStatusBar.SetText(_T("����Ԥ���ɹ���"), 1, 0);
            SetDlgItemText(IDC_BUTTON_PREVIEW_VIDEO, _T("ȡ��Ԥ��"));
        } else {
            _wndStatusBar.SetText(_T("����Ԥ��ʧ�ܣ�"), 1, 0);
            MessageBox(_T("Ԥ��ʧ�ܣ�"));
        }
    } else {
        if (0 == _rtc_video_interface->UnPreviewCamera()) {
            _wndStatusBar.SetText(_T("ֹͣ����Ԥ����"), 1, 0);
            SetDlgItemText(IDC_BUTTON_PREVIEW_VIDEO, _T("Ԥ��"));
        } else {
            _wndStatusBar.SetText(_T("ȡ������Ԥ��ʧ�ܣ�"), 1, 0);
            MessageBox(_T("ȡ��Ԥ��ʧ�ܣ�"));
        }
        GetDlgItem(IDC_STATIC_VIDEO_PREVIEW)->Invalidate();
    }
}

BOOL CRtcDemoDlg::PreTranslateMessage(MSG* pMsg)
{
    return __super::PreTranslateMessage(pMsg);
}

CRect&& CRtcDemoDlg::GetRenderWndPos()
{
    CRect parent_rc, child_tc;
    GetDlgItem(IDC_STATIC_PLAY)->GetClientRect(&parent_rc);
    child_tc.left  = 0;
    child_tc.top = DISPLAY_NAME_HEIGHT;
    child_tc.right = parent_rc.Height();
    child_tc.top = parent_rc.Height();
    return std::move(child_tc);
}

void CRtcDemoDlg::AdjustRenderWndPos()
{
    int pos_x(0);
    CRect parent_rc;
    GetDlgItem(IDC_STATIC_PLAY)->GetClientRect(&parent_rc);
    for (auto itor : _user_stream_map) {
        if (itor.second.render_wnd_ptr
            && itor.second.render_wnd_ptr->GetParent() == GetDlgItem(IDC_STATIC_PLAY)) {

            CRect dest_rc(pos_x,
                parent_rc.top + DISPLAY_NAME_HEIGHT,
                pos_x + parent_rc.Height(),
                parent_rc.Height());
            itor.second.render_wnd_ptr->MoveWindow(&dest_rc);

            CRect dest_rc2(pos_x,
                parent_rc.top,
                pos_x + parent_rc.Height(),
                DISPLAY_NAME_HEIGHT);
            itor.second.display_name_ptr->MoveWindow(&dest_rc2);

            TRACE("%s, left:%d, top:%d, right:%d, botton:%d\n", 
                __FUNCTION__, dest_rc.left, dest_rc.top, dest_rc.right, dest_rc.bottom);
            pos_x += parent_rc.Height();
        }
    }
}

void CRtcDemoDlg::OnBnClickedBtnKickout()
{
    // TODO: Add your control notification handler code here
    int index = _user_list_ctrl.GetSelectionMark();
    if (index == -1) {
        MessageBox(_T("��ѡ��Ҫ�߳����û���"));
        return;
    }
    //��ѡ����û���ǰû�з���ý����
    CString user_id = _user_list_ctrl.GetItemText(index, 0);

    if (_rtc_room_interface) {
        _rtc_room_interface->KickoutUser(unicode2utf(user_id.GetBuffer()).c_str());
    }
}

void CRtcDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_RECORD) {
        int old_pos(0);
        int pos = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_RECORD))->GetPos();
        if (_rtc_audio_interface) {
            old_pos = _rtc_audio_interface->GetAudioVolume(AudioDeviceInfo::adt_record);

            if (pos == 0) {
                _rtc_audio_interface->SetAudioMuteFlag(AudioDeviceInfo::adt_record, true);
            } else {
                _rtc_audio_interface->SetAudioMuteFlag(AudioDeviceInfo::adt_record, false);

                if (_rtc_audio_interface->SetAudioVolume(AudioDeviceInfo::adt_record, pos) < 0) {
                    MessageBox(_T("����¼������ʧ�ܣ�"));
                }
            }
        }
    } else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_PLAYOUT) {
        int old_pos(0);
        int pos = ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAYOUT))->GetPos();
        if (_rtc_audio_interface) {
            old_pos = _rtc_audio_interface->GetAudioVolume(AudioDeviceInfo::adt_playout);

            if (pos == 0) {
                _rtc_audio_interface->SetAudioMuteFlag(AudioDeviceInfo::adt_playout, true);
            } else {
                _rtc_audio_interface->SetAudioMuteFlag(AudioDeviceInfo::adt_playout, false);

                if (_rtc_audio_interface->SetAudioVolume(AudioDeviceInfo::adt_playout, pos) < 0) {
                    MessageBox(_T("����¼������ʧ�ܣ�"));
                }
            }
        }
    }

    __super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CRtcDemoDlg::OnBnClickedCheckMuteAudio()
{
    // TODO: Add your control notification handler code here
    int check_flag = ((CButton*)GetDlgItem(IDC_CHECK_MUTE_AUDIO))->GetCheck();
    if (_rtc_room_interface) {
        _rtc_room_interface->MuteAudio(check_flag > 0 ? false : true);
    }
}

void CRtcDemoDlg::OnBnClickedCheckMuteVideo()
{
    // TODO: Add your control notification handler code here
    int check_flag = ((CButton*)GetDlgItem(IDC_CHECK_MUTE_VIDEO))->GetCheck();
    if (_rtc_room_interface) {
        _rtc_room_interface->MuteVideo(check_flag > 0 ? false : true);
    }
}

void CRtcDemoDlg::OnCbnSelchangeComboPlayout()
{
    // TODO: Add your control notification handler code here
    int audio_playout_device_index = ((CComboBox *)GetDlgItem(IDC_COMBO_PLAYOUT))->GetCurSel();
    if (audio_playout_device_index >= 0 && _rtc_audio_interface) {
        qiniu::AudioDeviceSetting audio_setting;
        audio_setting.device_index = audio_playout_device_index;
        audio_setting.device_type = qiniu::AudioDeviceSetting::wdt_DefaultDevice;

        int ret = _rtc_audio_interface->SetPlayoutDevice(audio_setting);
        if (ret != QNRTC_OK) {
            std::thread([this]() {
                MessageBox(_T("������Ƶ����豸ʧ�ܣ��������������Է�������������û�а�����ָ�����豸���в��ţ�"));
            }).detach();
        }
    }
}

size_t WriteBuffer(void *src_, size_t src_size_, size_t blocks_, void *param_)
{
    string *str = (string*)(param_);
    str->append((char *)src_, src_size_ * blocks_);

    return str->size();
}

int CRtcDemoDlg::GetRoomToken(const string room_name_, const string user_id_, string& token_)
{
    if (room_name_.empty() || user_id_.empty()) {
        return -1;
    }
    curl_global_init(CURL_GLOBAL_ALL);
    auto curl = curl_easy_init();

    // set options
    char url_buf[1024] = { 0 };
    string tmp_uid = user_id_;
    // ����˺�����Ĭ�����ƣ�user id �а��� admin ��ӵ�к�����Ȩ��
    if (strstr(strlwr(const_cast<char*>(tmp_uid.c_str())), "admin")) {
        snprintf(url_buf,
            sizeof(url_buf),
            "http://api-demo.qnsdk.com/v1/rtc/token/admin/app/d8lk7l4ed/room/%s/user/%s",
            room_name_.c_str(),
            user_id_.c_str());
        _contain_admin_flag = true;
    } else {
        snprintf(url_buf,
            sizeof(url_buf),
            "http://api-demo.qnsdk.com/v1/rtc/token/app/d8lk7l4ed/room/%s/user/%s",
            room_name_.c_str(),
            user_id_.c_str());
        _contain_admin_flag = false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBuffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &token_);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    // send request now
    int status(0);
    CURLcode result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        long code;
        result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (result == CURLE_OK) {
            if (code != 200) {
                status = -2; // server auth failed
            } else {
                status = 0; //success
            }
        } else {
            status = -3; //connect server timeout
        }
    } else {
        status = -3; //connect server timeout
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return status;
}

std::tuple<int, int> CRtcDemoDlg::FindBestVideoSize(const CameraCapabilityVec& camera_cap_vec_)
{
    if (camera_cap_vec_.empty()) {
        return{ 0,0 };
    }
    // �߿�����
    float wh_ratio = 1.0f * 3 / 4;
    int dest_width(0), dest_height(0);
    for (auto itor : camera_cap_vec_) {
        if ((1.0f * itor.height / itor.width) == wh_ratio) {
            if (itor.width >= 480) {
                dest_width = itor.width;
                dest_height = itor.height;
            }
        }
    }
    if (dest_width == 0 || dest_height == 0) {
        dest_width = camera_cap_vec_.back().width;
        dest_height = camera_cap_vec_.back().height;
    }
    return std::make_tuple(dest_width, dest_height);
}

void CRtcDemoDlg::AdjustMergeStreamPosition()
{
    lock_guard<recursive_mutex> lock_(_mutex);
    int user_num(0), pos_num(0);
    for (auto&& itor : _user_stream_map) {
        if (itor.first.compare(unicode2utf(_user_id.GetBuffer())) == 0) {
            continue;
        }
        ++user_num;
        pos_num = 0;
        for (int y(2); y >= 0; --y) {
            for (int x(2); x >= 0; --x) {
                ++pos_num;
                if (pos_num == user_num) {
                    _rtc_room_interface->SetMergeStreamLayout(
                        itor.first, 
                        Canvas_Width / 3 * x, 
                        Canvas_Height / 3 * y,
                        pos_num, 
                        Canvas_Width / 3, 
                        Canvas_Height / 3, 
                        true);
                }
            }
        }
    }
}

void CRtcDemoDlg::ReadConfigFile()
{
    ifstream is("config");
    if (is.bad()) {
        return;
    }
    char room_buf[128] = { 0 };
    char user_buf[128] = { 0 };
    if (!is.getline(room_buf, 128)) {
        return;
    }
    if (!is.getline(user_buf, 128)) {
        return;
    }
    SetDlgItemText(IDC_EDIT_ROOM_ID, utf2unicode(room_buf).c_str());
    SetDlgItemText(IDC_EDIT_PLAYER_ID, utf2unicode(user_buf).c_str());
    is.close();
}

void CRtcDemoDlg::WriteConfigFile()
{
    ofstream os("config");
    if (os.bad()) {
        return;
    }
    os.clear();
    string room_name = unicode2utf(_room_name.GetBuffer());
    string user_id = unicode2utf(_user_id.GetBuffer());
    os.write(room_name.c_str(), room_name.size());
    os.write("\n", 1);
    os.write(user_id.c_str(), user_id.size());
    os.close();
}

void CRtcDemoDlg::InitStatusBar()
{
    _wndStatusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);
    int strPartDim[3] = { 200, 1150, -1 };
    _wndStatusBar.SetParts(3, strPartDim);
    //����״̬���ı�  
    _wndStatusBar.SetText(_T("ͨ��ʱ����00:00::00"), 0, 0);
    _wndStatusBar.SetText(_T("����״̬"), 1, 0);
    _wndStatusBar.SetText(utf2unicode(GetAppVersion()).c_str(), 2, 0);
}

void CRtcDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == UPDATE_TIME_TIMER_ID) {
        chrono::seconds df_time 
            = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - _start_time);
        int hour = df_time.count() / 3600;
        int minute = df_time.count() % 3600 / 60;
        int sec = df_time.count() % 3600 % 60;
        wchar_t time_buf[128] = { 0 };
        wsprintf(time_buf, 
            _T("����ʱ����%02d:%02d:%02d"), 
            hour,
            minute,
            sec
        );
        _wndStatusBar.SetText(time_buf, 0, 0);
    } else {
        KillTimer(nIDEvent);

        lock_guard<recursive_mutex> lock_(_mutex);
        if (_call_function_vec.empty()) {
            KillTimer(nIDEvent);
            return;
        }
        while (!_call_function_vec.empty()) {
            _call_function_vec.front()();
            _call_function_vec.pop_front();
        }
        AdjustRenderWndPos();
    }
}