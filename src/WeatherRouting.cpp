/***************************************************************************
 *
 * Project:  OpenCPN Weather Routing plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier                                 *
 *   sean@depagnier.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#include <wx/wx.h>
#include <wx/imaglist.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "../../../include/tinyxml.h"

#include "Utilities.h"
#include "Boat.h"
#include "BoatDialog.h"
#include "ConfigurationBatchDialog.h"
#include "RouteMapOverlay.h"
#include "weather_routing_pi.h"
#include "WeatherRouting.h"
#include "PlotDialog.h"
#include "AboutDialog.h"

/* XPM */
static const char *eye[]={
"20 20 7 1",
". c none",
"# c #000000",
"a c #333333",
"b c #666666",
"c c #999999",
"d c #cccccc",
"e c #ffffff",
"....................",
"....................",
"....................",
"....................",
".......######.......",
".....#aabccb#a#.....",
"....#deeeddeebcb#...",
"..#aeeeec##aceaec#..",
".#bedaeee####dbcec#.",
"#aeedbdabc###bcceea#",
".#bedad######abcec#.",
"..#be#d######dadb#..",
"...#abac####abba#...",
".....##acbaca##.....",
".......######.......",
"....................",
"....................",
"....................",
"....................",
"...................."};

enum {VISIBLE=0, NAME, START, END, TIME, DISTANCE, AVGSPEED, STATE};

WeatherRouting::WeatherRouting(wxWindow *parent, weather_routing_pi &plugin)
    : WeatherRoutingBase(parent), m_ConfigurationDialog(this, plugin),
      m_SettingsDialog(this), m_StatisticsDialog(this),
      m_bRunning(false), m_bSkipUpdateCurrentItem(false)
{
    m_ConfigurationDialog.Hide();

    m_SettingsDialog.LoadSettings();

    m_SettingsDialog.Hide();

    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/PlugIns/WeatherRouting" ) );

    wxImageList *imglist = new wxImageList(20, 20, true, 1);
    imglist->Add(wxBitmap(eye));
    m_lWeatherRoutes->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

    m_lWeatherRoutes->InsertColumn(VISIBLE, _T(""));
    m_lWeatherRoutes->SetColumnWidth(0, 28);

    m_lWeatherRoutes->InsertColumn(NAME, _("Name"));
    m_lWeatherRoutes->InsertColumn(START, _("Start"));
    m_lWeatherRoutes->InsertColumn(END, _("End"));
    m_lWeatherRoutes->InsertColumn(TIME, _("Time"));
    m_lWeatherRoutes->InsertColumn(DISTANCE, _("Distance"));
    m_lWeatherRoutes->InsertColumn(AVGSPEED, _("Average Speed"));
    m_lWeatherRoutes->InsertColumn(STATE, _("State"));

    m_default_configuration_path = weather_routing_pi::StandardPath()
        + _("WeatherRoutingConfiguration.xml");
    if(!OpenXML(m_default_configuration_path, false)) {
        AddConfiguration(DefaultConfiguration());

        m_lWeatherRoutes->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
//        m_ConfigurationDialog.Show();
    }

    wxPoint p = GetPosition();
    pConf->Read ( _T ( "DialogX" ), &p.x, p.x);
    pConf->Read ( _T ( "DialogY" ), &p.y, p.y);
    SetPosition(p);

    wxSize s = GetSize();
    pConf->Read ( _T ( "DialogWidth" ), &s.x, s.x);
    pConf->Read ( _T ( "DialogHeight" ), &s.y, s.y);
    SetSize(s);


    /* periodically check for updates from computation thread */
    m_tCompute.Connect(wxEVT_TIMER, wxTimerEventHandler
                       ( WeatherRouting::OnComputationTimer ), NULL, this);
}

WeatherRouting::~WeatherRouting( )
{
    wxFileConfig *pConf = GetOCPNConfigObject();
    pConf->SetPath ( _T( "/PlugIns/WeatherRouting" ) );

    wxPoint p = GetPosition();
    pConf->Write ( _T ( "DialogX" ), p.x);
    pConf->Write ( _T ( "DialogY" ), p.y);

    wxSize s = GetSize();
    pConf->Write ( _T ( "DialogWidth" ), s.x);
    pConf->Write ( _T ( "DialogHeight" ), s.y);

    SaveXML(m_default_configuration_path);
}

void WeatherRouting::Render(ocpnDC &dc, PlugIn_ViewPort &vp)
{
    if(!dc.GetDC()) {
        glPushAttrib(GL_LINE_BIT | GL_ENABLE_BIT | GL_HINT_BIT ); //Save state
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    }

    wxDateTime time = m_ConfigurationDialog.m_GribTimelineTime;
    if(!time.IsValid())
        time = wxDateTime::Now();

    RouteMapOverlay *routemapoverlay = CurrentRouteMap();
    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        RouteMapOverlay *rmo =
            reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
        if(/*rmo != routemapoverlay && */rmo->m_bEndRouteVisible)
            rmo->Render(time, m_SettingsDialog, dc, vp, true);
    }

    if(routemapoverlay)
        routemapoverlay->Render(time, m_SettingsDialog, dc, vp, false);

    if(!dc.GetDC())
        glPopAttrib();
}

void WeatherRouting::OnConfiguration()
{
    if(CurrentRouteMap(true))
        m_ConfigurationDialog.Show();
}

void WeatherRouting::OnWeatherRouteSelected( wxListEvent& event )
{
    GetParent()->Refresh();
    if(CurrentRouteMap()) {
        m_bSkipUpdateCurrentItem = true;
        m_ConfigurationDialog.SetConfiguration(CurrentRouteMap()->GetConfiguration());
        m_bSkipUpdateCurrentItem = false;
        m_StatisticsDialog.SetRouteMapOverlay(CurrentRouteMap());
    } else
        m_ConfigurationDialog.Hide();
}

void WeatherRouting::OnWeatherRoutesListLeftDown(wxMouseEvent &event)
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long index = m_lWeatherRoutes->HitTest(pos, flags);
    
    //    Clicking Visibility column?
    if (index > -1 && event.GetX() < m_lWeatherRoutes->GetColumnWidth(0))
    {
        // Process the clicked item
        RouteMapOverlay *routemapoverlay =
            reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(index)));
        routemapoverlay->m_bEndRouteVisible = !routemapoverlay->m_bEndRouteVisible;
        UpdateItem(index);
        RequestRefresh( GetParent() );
    }

    // Allow wx to process...
    event.Skip();
}

int debugcnt, debuglimit = 1, debugsize = 2;
void WeatherRouting::OnCompute ( wxCommandEvent& event )
{
    if(m_bRunning)
        Stop();
    else
        Start();
}

#define FAIL(X) do { error = X; goto failed; } while(0)
void WeatherRouting::OnOpen( wxCommandEvent& event )
{
    wxString error;
    wxFileDialog openDialog
        ( this, _( "Select Configuration" ), _("~"), wxT ( "" ),
          wxT ( "XML files (*.xml)|*.XML;*.xml|All files (*.*)|*.*" ),
          wxFD_OPEN  );

    if( openDialog.ShowModal() == wxID_OK )
        OpenXML(openDialog.GetPath());
}

void WeatherRouting::OnSave( wxCommandEvent& event )
{
    wxString error;
    wxFileDialog saveDialog
        ( this, _( "Select Configuration" ), _("~"), wxT ( "" ),
          wxT ( "XML files (*.xml)|*.XML;*.xml|All files (*.*)|*.*" ),
          wxFD_SAVE  );

    if( saveDialog.ShowModal() == wxID_OK )
        SaveXML(saveDialog.GetPath());
}

void WeatherRouting::OnClose( wxCommandEvent& event )
{
    Hide();
}

void WeatherRouting::OnNew( wxCommandEvent& event )
{
    RouteMapConfiguration configuration;
    if(CurrentRouteMap())
        configuration = CurrentRouteMap()->GetConfiguration();
    else
        configuration = DefaultConfiguration();

    for(int idx = 0;;idx++) {
        wxString name = configuration.Name;
        wxString n1 = name.BeforeLast('-'), n2 = name.AfterLast('-');
        long l;
        if(n1.size() && n2.ToLong(&l))
            name = n1;

        if(idx > 0)
            name += wxString::Format(_T("-%d"), idx);
        for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
            RouteMapOverlay *routemapoverlay =
                reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
            RouteMapConfiguration c = routemapoverlay->GetConfiguration();
            if(c.Name == name)
                goto outer_continue;
        }
        configuration.Name = name;
        break;
    outer_continue:;
    }

    AddConfiguration(configuration);
}

void WeatherRouting::OnBatch( wxCommandEvent& event )
{
    if(CurrentRouteMap(true)) {
        wxMessageDialog mdlg(this, _T("This function is Unimplemented"), _("OpenCPN Alert"), wxOK);
        mdlg.ShowModal();

        ConfigurationBatchDialog dlg(this, CurrentRouteMap()->GetConfiguration());
        dlg.ShowModal();
    }
}

void WeatherRouting::OnExport ( wxCommandEvent& event )
{
    if(CurrentRouteMap(true))
        Export(*CurrentRouteMap());
}

void WeatherRouting::OnDelete( wxCommandEvent& event )
{
    long index = m_lWeatherRoutes->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index < 0) return;

    RouteMapOverlay *routemapoverlay = reinterpret_cast<RouteMapOverlay*>
        (wxUIntToPtr(m_lWeatherRoutes->GetItemData(index)));
    DeleteRouteMap(routemapoverlay);
    m_lWeatherRoutes->DeleteItem(index);
    GetParent()->Refresh();
}

void WeatherRouting::OnFilter( wxCommandEvent& event )
{
    wxMessageDialog mdlg(this, _("Unimplemented"), _("OpenCPN Alert"), wxOK);
    mdlg.ShowModal();
}

void WeatherRouting::OnReset ( wxCommandEvent& event )
{
    Reset();
}

void WeatherRouting::OnExportAll( wxCommandEvent& event )
{
    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++)
        Export(*reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i))));
}

void WeatherRouting::OnDeleteAll( wxCommandEvent& event )
{
    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        RouteMapOverlay *routemapoverlay =
            reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
        DeleteRouteMap(routemapoverlay);
    }
    m_lWeatherRoutes->DeleteAllItems();
    GetParent()->Refresh();
}

void WeatherRouting::OnSettings( wxCommandEvent& event )
{
    m_SettingsDialog.LoadSettings();

    if(m_SettingsDialog.ShowModal() == wxID_OK) {
        m_SettingsDialog.SaveSettings();

        for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
            RouteMapOverlay *routemapoverlay =
                reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
            routemapoverlay->m_UpdateOverlay = true;
        }

        GetParent()->Refresh();
    }
}

void WeatherRouting::OnStatistics( wxCommandEvent& event )
{
    m_StatisticsDialog.Show();
}

void WeatherRouting::OnPlot ( wxCommandEvent& event )
{
    if(CurrentRouteMap(true)) {
        std::list<PlotData> plotdata = CurrentRouteMap()->GetPlotData();
        PlotDialog plotdialog(this, plotdata);
        plotdialog.ShowModal();
    }
}

void WeatherRouting::OnInformation ( wxCommandEvent& event )
{
    InformationDialog dlg(this);
    wxString infolocation = *GetpSharedDataLocation()
        + _("plugins/weather_routing/data/WeatherRoutingInformation.html");
    if(dlg.m_htmlInformation->LoadFile(infolocation))
        dlg.ShowModal();
    else {
        wxMessageDialog mdlg(this, _("Failed to load file:\n") + infolocation,
                             _("OpenCPN Alert"), wxOK | wxICON_ERROR);
        mdlg.ShowModal();
    }
}

void WeatherRouting::OnAbout ( wxCommandEvent& event )
{
    AboutDialog dlg(this);
    dlg.ShowModal();
}

void WeatherRouting::OnComputationTimer( wxTimerEvent & )
{
#if 0
    /* complete hack to make window size right.. not sure why we can't call fit earlier,
       but it doesn't work */
    static int fitstartup = 2;
    if(IsShown() && fitstartup) {
        Fit();
        m_SettingsDialog.Fit();
        fitstartup--;
    }
#endif

    for(std::list<RouteMapOverlay*>::iterator it = m_RunningRouteMaps.begin();
        it != m_RunningRouteMaps.end(); it++) {
        RouteMapOverlay *routemapoverlay = *it;
        if(!routemapoverlay->Running()) {
            UpdateRouteMap(routemapoverlay);

            std::list<RouteMapOverlay*>::iterator prev = it;
            prev--;
            m_RunningRouteMaps.erase(it);
            it = prev;

            m_gProgress->SetValue(m_RoutesToRun - m_WaitingRouteMaps.size() - m_RunningRouteMaps.size());
            continue;
        }

        /* get a new grib for the route map if needed */
        if(routemapoverlay->NeedsGrib()) {
            m_RouteMapOverlayNeedingGrib = routemapoverlay;
            routemapoverlay->RequestGrib(routemapoverlay->NewTime());
            m_RouteMapOverlayNeedingGrib = NULL;
        }
    }

    if((int)m_RunningRouteMaps.size() < m_SettingsDialog.m_sConcurrentThreads->GetValue()
       && m_WaitingRouteMaps.size()) {
        RouteMapOverlay *routemapoverlay = m_WaitingRouteMaps.front();
        m_WaitingRouteMaps.pop_front();
        routemapoverlay->Start();
        UpdateRouteMap(routemapoverlay);

        m_RunningRouteMaps.push_back(routemapoverlay);
    }
        
    static int cycles; /* don't refresh all the time */
    if(++cycles > 25 && CurrentRouteMap() && CurrentRouteMap()->Updated()) {
        cycles = 0;
        
        m_StatisticsDialog.SetRunTime(m_RunTime += wxDateTime::Now() - m_StartTime);
        m_StartTime = wxDateTime::Now();
            
        GetParent()->Refresh();
    }

    if(m_RunningRouteMaps.size()) {
        m_tCompute.Start(100, true);
        return;
    }

    Stop();
}

bool WeatherRouting::OpenXML(wxString filename, bool reportfailure)
{
    TiXmlDocument doc;
    wxString error;
    if(!doc.LoadFile(filename.mb_str()))
        FAIL(_("Failed to load file."));
    else {
        TiXmlHandle root( doc.RootElement() );
        if(strcmp(doc.RootElement()->Value(), "OCPNWeatherRoutingConfiguration"))
            FAIL(_("Invalid xml file"));

        for(TiXmlElement* e = root.FirstChild().Element(); e; e = e->NextSiblingElement()) {
            if(!strcmp(e->Value(), "Configuration")) {
                RouteMapConfiguration configuration;
                configuration.Name = wxString::FromUTF8(e->Attribute("Name"));
                configuration.StartLat = AttributeDouble(e, "StartLat", 0);
                configuration.StartLon = AttributeDouble(e, "StartLon", 0);
                wxDateTime date;
                date.ParseDate(wxString::FromUTF8(e->Attribute("StartDate")));
                wxDateTime time;
                time.ParseTime(wxString::FromUTF8(e->Attribute("StartTime")));
                if(date.IsValid()) {
                    if(time.IsValid()) {
                        date.SetHour(time.GetHour());
                        date.SetMinute(time.GetMinute());
                        date.SetSecond(time.GetSecond());
                    }
                    configuration.StartTime = date;
                } else
                    configuration.StartTime = wxDateTime::Now();

                configuration.EndLat = AttributeDouble(e, "EndLat", 0);
                configuration.EndLon = AttributeDouble(e, "EndLon", 0);
                configuration.dt = AttributeDouble(e, "dt", 0);
                
                configuration.boatFileName = wxString::FromUTF8(e->Attribute("Boat"));
                
                configuration.MaxDivertedCourse = AttributeDouble(e, "MaxDivertedCourse", 180);
                configuration.MaxWindKnots = AttributeDouble(e, "MaxWindKnots", 100);
                configuration.MaxSwellMeters = AttributeDouble(e, "MaxSwellMeters", 20);
                configuration.MaxLatitude = AttributeDouble(e, "MaxLatitude", 90);
                configuration.MaxTacks = AttributeDouble(e, "MaxTacks", -1);
                configuration.TackingTime = AttributeDouble(e, "TackingTime", 0);
                
                configuration.UseGrib = AttributeBool(e, "UseGrib", true);
                configuration.UseClimatology = AttributeBool(e, "UseClimatology", true);
                configuration.AllowDataDeficient = AttributeBool(e, "AllowDataDeficient", false);
                configuration.DetectLand = AttributeBool(e, "DetectLand", true);
                configuration.Currents = AttributeBool(e, "Currents", true);
                configuration.InvertedRegions = AttributeBool(e, "InvertedRegions", false);
                configuration.Anchoring = AttributeBool(e, "Anchoring", false);

                wxString degreesteps = wxString::FromUTF8(e->Attribute("DegreeSteps"));
                while(degreesteps.size()) {
                    double step;
                    if(degreesteps.BeforeFirst(';').ToDouble(&step))
                        configuration.DegreeSteps.push_back(step);
                    degreesteps = degreesteps.AfterFirst(';');
                }
 
                AddConfiguration(configuration);
            } else
                FAIL(_("Unrecognized xml node"));
        }
    }

    return true;
failed:

    if(reportfailure) {
        wxMessageDialog mdlg(this, error, _("Weather Routing"), wxOK | wxICON_ERROR);
        mdlg.ShowModal();
    }
    return false;
}

void WeatherRouting::SaveXML(wxString filename)
{
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "utf-8", "" );
    doc.LinkEndChild( decl );

    TiXmlElement * root = new TiXmlElement( "OCPNWeatherRoutingConfiguration" );
    doc.LinkEndChild( root );

    char version[24];
    sprintf(version, "%d.%d", PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    root->SetAttribute("version", version);
    root->SetAttribute("creator", "Opencpn Weather Routing plugin");

    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        TiXmlElement *c = new TiXmlElement( "Configuration" );

        RouteMapConfiguration configuration =
            reinterpret_cast<RouteMap*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)))
            ->GetConfiguration();

        c->SetAttribute("Name", configuration.Name.mb_str());
        c->SetAttribute("StartLat", wxString::Format(_T("%.5f"), configuration.StartLat).mb_str());
        c->SetAttribute("StartLon", wxString::Format(_T("%.5f"), configuration.StartLon).mb_str());
        c->SetAttribute("StartDate", configuration.StartTime.FormatDate().mb_str());
        c->SetAttribute("StartTime", configuration.StartTime.FormatTime().mb_str());
        c->SetAttribute("EndLat", wxString::Format(_T("%.5f"), configuration.EndLat).mb_str());
        c->SetAttribute("EndLon", wxString::Format(_T("%.5f"), configuration.EndLon).mb_str());
        c->SetAttribute("dt", configuration.dt);

        c->SetAttribute("Boat", configuration.boatFileName.ToUTF8());

        c->SetAttribute("MaxDivertedCourse", configuration.MaxDivertedCourse);
        c->SetAttribute("MaxWindKnots", configuration.MaxWindKnots);
        c->SetAttribute("MaxSwellMeters", configuration.MaxSwellMeters);
        c->SetAttribute("MaxLatitude", configuration.MaxLatitude);
        c->SetAttribute("MaxTacks", configuration.MaxTacks);
        c->SetAttribute("TackingTime", configuration.TackingTime);

        c->SetAttribute("UseGrib", configuration.UseGrib);
        c->SetAttribute("UseClimatology", configuration.UseClimatology);
        c->SetAttribute("AllowDataDeficient", configuration.AllowDataDeficient);
        c->SetAttribute("DetectLand", configuration.DetectLand);
        c->SetAttribute("Currents", configuration.Currents);
        c->SetAttribute("InvertedRegions", configuration.InvertedRegions);
        c->SetAttribute("Anchoring", configuration.Anchoring);

        wxString degreesteps;
        for(std::list<double>::iterator it = configuration.DegreeSteps.begin();
            it != configuration.DegreeSteps.end(); it++)
            degreesteps += wxString::Format(_T("%.1f;"), *it);
        c->SetAttribute("DegreeSteps", degreesteps.mb_str());
        
        root->LinkEndChild(c);
    }

    if(!doc.SaveFile(filename.mb_str())) {
        wxMessageDialog mdlg(this, _("Failed to save xml file: ") + filename,
                             _("Weather Routing"), wxOK | wxICON_ERROR);
        mdlg.ShowModal();
    }
}

void WeatherRouting::AddConfiguration(RouteMapConfiguration configuration)
{
    RouteMapOverlay *routemapoverlay = new RouteMapOverlay;
    routemapoverlay->SetConfiguration(configuration);
    routemapoverlay->Reset();

    wxListItem item;
    int count = m_lWeatherRoutes->GetItemCount();
    item.SetId(count);
    item.SetData(routemapoverlay);

    long index = m_lWeatherRoutes->InsertItem(item);
    UpdateItem(index);
}

void WeatherRouting::UpdateRouteMap(RouteMapOverlay *routemapoverlay)
{
    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        RouteMap *rmo =
            reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
        if(rmo == routemapoverlay) {
            UpdateItem(i);
            return;
        }
    }
}

void WeatherRouting::UpdateItem(long index)
{
    RouteMapOverlay *routemapoverlay = reinterpret_cast<RouteMapOverlay*>
        (wxUIntToPtr(m_lWeatherRoutes->GetItemData(index)));
    RouteMapConfiguration configuration = routemapoverlay->GetConfiguration();

    m_lWeatherRoutes->SetItemImage(index, routemapoverlay->m_bEndRouteVisible ? 0 : -1);
    m_lWeatherRoutes->SetItem(index, NAME, configuration.Name);
    m_lWeatherRoutes->SetItem(index, START, wxString::Format(_T("%.2f, %.2f : "),
                                                             configuration.StartLat,
                                                             configuration.StartLon)
                              + configuration.StartTime.Format(_T("%x %H:%M")));
    m_lWeatherRoutes->SetColumnWidth(START, wxLIST_AUTOSIZE);

    m_lWeatherRoutes->SetItem(index, END, wxString::Format(_T("%.2f, %.2f"),
                                                           configuration.EndLat,
                                                           configuration.EndLon));
    m_lWeatherRoutes->SetColumnWidth(END, wxLIST_AUTOSIZE);

    wxDateTime enddate = routemapoverlay->EndDate();
    if(enddate.IsValid()) {
//        m_stEndDate->SetLabel(enddate.FormatDate());
//        m_stEndHour->SetLabel(enddate.Format(_T("%H")));

        wxTimeSpan span = enddate - routemapoverlay->StartTime();
        int days = span.GetDays();
        span -= wxTimeSpan::Days(days);
        int hours = span.GetHours();
        span -= wxTimeSpan::Hours(hours);
        int minutes = (double)span.GetSeconds().ToLong()/60.0;

        m_lWeatherRoutes->SetItem(index, TIME,
                                  (days ? wxString::Format(_T("%dd "), days) : _T("")) +
                                  wxString::Format(_T("%02d:%02d"), hours, minutes));
    } else
        m_lWeatherRoutes->SetItem(index, TIME, _("N/A"));
    m_lWeatherRoutes->SetColumnWidth(TIME, wxLIST_AUTOSIZE);

    double distance, avgspeed, percentage_upwind;
    routemapoverlay->RouteInfo(distance, avgspeed, percentage_upwind);

    m_lWeatherRoutes->SetItem(index, DISTANCE, isnan(distance) ? _T("N/A") :
                              wxString::Format(_T("%.0f nm"), distance));
    m_lWeatherRoutes->SetItem(index, AVGSPEED, isnan(avgspeed) ? _T("N/A") :
                              wxString::Format(_T("%.1f knots"), avgspeed));
#if 0
    m_lWeatherRoutes->SetItem(index, PERCENTAGE_UPWIND, isnan(percentage_upwind) ? _T("N/A") :
                              wxString::Format(_T("%.0f%%"), percentage_upwind));
#endif

    wxString state;
    if(routemapoverlay->Running())
        state = _("Computing...");
    else {
        state += routemapoverlay->Finished() ? _T("") : _("Not");
        state += _T(" ");
        state += _("Finished");
        state += _T(" ");
        state += _("Destination");
        state += _T(" ");
        state += routemapoverlay->ReachedDestination() ? _T("") : _("Not");
        state += _T(" ");
        state += _("Reached");
        if(routemapoverlay->GribFailed()) {
            state += _T(" ");
            state += _("Grib Failed");
        }
    }

    m_lWeatherRoutes->SetItem(index, STATE, state);
    m_lWeatherRoutes->SetColumnWidth(STATE, wxLIST_AUTOSIZE);
}

void WeatherRouting::UpdateCurrentItem(RouteMapConfiguration configuration)
{
    if(m_bSkipUpdateCurrentItem)
        return;

    if(CurrentRouteMap())
        CurrentRouteMap()->SetConfiguration(configuration);

    long index = m_lWeatherRoutes->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index < 0) return;

    UpdateItem(index);
}

RouteMapOverlay *WeatherRouting::CurrentRouteMap(bool messagedialog)
{
    long index = m_lWeatherRoutes->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    RouteMapOverlay *routemapoverlay = NULL;
    if (index >= 0)
        routemapoverlay = reinterpret_cast<RouteMapOverlay*>
            (wxUIntToPtr(m_lWeatherRoutes->GetItemData(index)));

    if(messagedialog && !routemapoverlay) {
        wxMessageDialog mdlg(this, _("No Weather Route selected"),
                             _("Weather Routing"), wxOK | wxICON_WARNING);
        mdlg.ShowModal();
    }

    return routemapoverlay;
}

void WeatherRouting::Export(RouteMapOverlay &routemapoverlay)
{
    std::list<PlotData> plotdata = routemapoverlay.GetPlotData();

    if(plotdata.size() == 0) {
        wxMessageDialog mdlg(this, _("Empty Route, nothing to export\n"),
                             _("Weather Routing"), wxOK | wxICON_WARNING);
        mdlg.ShowModal();
        return;
    }

    PlugIn_Track* newTrack = new PlugIn_Track;
    newTrack->m_NameString = _("Weather Route");

    for(std::list<PlotData>::iterator it = plotdata.begin(); it != plotdata.end(); it++) {
        PlugIn_Waypoint*  newPoint = new PlugIn_Waypoint
            ((*it).lat, (*it).lon, _T("circle"), _("Weather Route Point"));

        newPoint->m_CreateTime = (*it).time;
        newTrack->pWaypointList->Append(newPoint);
    }

    AddPlugInTrack(newTrack);

    GetParent()->Refresh();
}

void WeatherRouting::Start()
{
    Reset();

    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        RouteMapOverlay *routemapoverlay =
            reinterpret_cast<RouteMapOverlay*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
        if(!routemapoverlay->Finished())
            m_WaitingRouteMaps.push_back(routemapoverlay);
    }
    m_RoutesToRun = m_WaitingRouteMaps.size();
    m_gProgress->SetRange(m_RoutesToRun);
    m_gProgress->SetValue(0);

    m_bRunning = true;
    m_bCompute->SetLabel(_( "&Stop" ));
    m_StartTime = wxDateTime::Now();
    m_tCompute.Start(100, true);
}

void WeatherRouting::Stop()
{
    for(std::list<RouteMapOverlay*>::iterator it = m_RunningRouteMaps.begin();
        it != m_RunningRouteMaps.end(); it++)
        (*it)->Stop();

    m_RunningRouteMaps.clear();
    m_WaitingRouteMaps.clear();

    m_gProgress->SetValue(0);

    m_bRunning = false;
    m_bCompute->SetLabel(_( "&Start" ));
    m_StatisticsDialog.SetRunTime(m_RunTime += wxDateTime::Now() - m_StartTime);
}

void WeatherRouting::Reset()
{
    if(m_bRunning)
        return;

    for(int i=0; i<m_lWeatherRoutes->GetItemCount(); i++) {
        RouteMap *routemap =
            reinterpret_cast<RouteMap*>(wxUIntToPtr(m_lWeatherRoutes->GetItemData(i)));
        routemap->Reset();
    }

    m_StatisticsDialog.SetRunTime(m_RunTime = wxTimeSpan(0));
    GetParent()->Refresh();
}

void WeatherRouting::DeleteRouteMap(RouteMapOverlay *routemapoverlay)
{
    for(std::list<RouteMapOverlay*>::iterator it = m_RunningRouteMaps.begin();
        it != m_RunningRouteMaps.end(); it++)
        if(*it == routemapoverlay) {
            m_WaitingRouteMaps.erase(it);
            break;
        }

    for(std::list<RouteMapOverlay*>::iterator it = m_RunningRouteMaps.begin();
        it != m_RunningRouteMaps.end(); it++)
        if(*it == routemapoverlay) {
            m_RunningRouteMaps.erase(it);
            break;
        }

    delete routemapoverlay;
}

RouteMapConfiguration WeatherRouting::DefaultConfiguration()
{
    RouteMapConfiguration configuration;

    configuration.Name = _("Untitled");
    configuration.StartLat = 0, configuration.StartLon = 0;
    configuration.StartTime = wxDateTime::Now();
    configuration.dt = 3600;
    configuration.EndLat = 0, configuration.EndLon = 0;
    
    configuration.boatFileName = weather_routing_pi::StandardPath() + _("Boat.xml");
    
    configuration.MaxDivertedCourse = 180;
    configuration.MaxWindKnots = 100;
    configuration.MaxSwellMeters = 20;
    configuration.MaxLatitude = 90;
    configuration.MaxTacks = -1;
    configuration.TackingTime = 0;
    
    configuration.UseGrib = configuration.UseClimatology = true;
    configuration.AllowDataDeficient = false;
    configuration.DetectLand = true;
    configuration.Currents = false;
    configuration.InvertedRegions = false;
    configuration.Anchoring = false;
        
    for(double d=45; d<=170; d+=5) {
        configuration.DegreeSteps.push_back(d);
        configuration.DegreeSteps.push_back(-d);
    }

    return configuration;
}