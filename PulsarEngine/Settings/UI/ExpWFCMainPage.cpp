#include <MarioKartWii/UI/Page/Other/GlobeSearch.hpp>
#include <MarioKartWii/RKSYS/RKSYSMgr.hpp>
#include <Settings/UI/ExpWFCMainPage.hpp>
#include <UI/UI.hpp>
#include <UI/PlayerCount.hpp>
#include <core/nw4r/lyt/Pane.hpp>

namespace Pulsar {
namespace UI {
//EXPANDED WFC, keeping WW button and just hiding it in case it is ever needed...

kmWrite32(0x8064b984, 0x60000000); //nop the InitControl call in the init func
kmWrite24(0x80899a36, 'PUL'); //8064ba38
kmWrite24(0x80899a5B, 'PUL'); //8064ba90

void ExpWFCMain::OnInit() {
    this->InitControlGroup(8); //5 controls usually + settings button (5) + playerCount (6) + rankInfo (7)
    WFCMainMenu::OnInit();
    this->AddControl(5, settingsButton, 0);

    this->settingsButton.Load(UI::buttonFolder, "Settings1P", "Settings", 1, 0, false);
    this->settingsButton.buttonId = 5;
    this->settingsButton.SetOnClickHandler(this->onSettingsClick, 0);
    this->settingsButton.SetOnSelectHandler(this->onButtonSelectHandler);

    this->AddControl(6, playerCount, 0);
    ControlLoader loader(&this->playerCount);
    loader.Load(UI::buttonFolder, "PlayerButton", "VRButton", nullptr);
    for (int i = 0; i < 4; ++i) {
        this->playerCount.positionAndscale[i].position.x = 0.0f;
    }
    this->playerCount.SetPosition(0.0f);

    this->AddControl(7, rankInfo, 0);
    ControlLoader rankLoader(&this->rankInfo);
    rankLoader.Load(UI::buttonFolder, "RankButton", "VRButton", nullptr);
    this->rankInfo.isHidden = true;

    this->topSettingsPage = SettingsPanel::id;
}

void ExpWFCMain::BeforeControlUpdate() {
    WFCMainMenu::BeforeControlUpdate();

    int nTotal = 0;
    PlayerCount::GetNumbersTotal(nTotal);

    Text::Info info;
    info.intToPass[0] = nTotal;
    this->playerCount.SetTextBoxMessage("go", BMG_PLAYER_COUNT, &info);

    this->rankInfo.isHidden = true;
}

void ExpWFCMain::OnSettingsButtonClick(PushButton& pushButton, u32 r5) {
    ExpSection::GetSection()->GetPulPage<SettingsPanel>()->prevPageId = PAGE_WFC_MAIN;
    this->nextPageId = static_cast<PageId>(this->topSettingsPage);
    this->EndStateAnimated(0, pushButton.GetAnimationFrameSize());
}

void ExpWFCMain::ExtOnButtonSelect(PushButton& button, u32 hudSlotId) {
    if(button.buttonId == 5) {
        u32 bmgId = BMG_SETTINGS_BOTTOM + 1;
        if(this->topSettingsPage == PAGE_VS_TEAMS_VIEW) bmgId += 1;
        else if(this->topSettingsPage == PAGE_BATTLE_MODE_SELECT) bmgId += 2;
        this->bottomText.SetMessage(bmgId, 0);
    }
    else this->OnButtonSelect(button, hudSlotId);
}

//ExpWFCModeSel
kmWrite32(0x8064c284, 0x38800001); //distance func

void ExpWFCModeSel::OnInit() {
    WFCModeSelect::OnInit();
    // this->manipulatorManager.SetGlobalHandler(START_PRESS, this->onStartPress, false, false);
}

void ExpWFCModeSel::BeforeControlUpdate() {
    WFCModeSelect::BeforeControlUpdate();

    int nVS, n200cc, nOTT, nIR, nBattle;
    PlayerCount::GetNumbers(nVS, n200cc, nOTT, nIR, nBattle);

    Text::Info info;
    info.intToPass[0] = nVS;
    this->vsButton.SetTextBoxMessage("go", BMG_PLAYER_COUNT, &info);

    info.intToPass[0] = nOTT;
    this->ottButton.SetTextBoxMessage("go", BMG_PLAYER_COUNT, &info);

    this->battleButton.SetTextBoxMessage("go", 0x1402, nullptr);

    // Update VR rating button
    RKSYS::Mgr* rksysMgr = RKSYS::Mgr::sInstance;
    u32 vr = 0;
    u32 br = 5000;
    if(rksysMgr->curLicenseId >= 0) {
        RKSYS::LicenseMgr& license = rksysMgr->licenses[rksysMgr->curLicenseId];
        vr = license.vr.points;
        br = license.br.points;
    }
    Text::Info vrInfo;
    vrInfo.intToPass[0] = this->battleButton.IsSelected() ? br : vr;
    u32 bmgId = this->battleButton.IsSelected() ? 0x6969 : 0x285e;
    this->vrButton.SetTextBoxMessage("go", bmgId, &vrInfo);

    // Force pane visibility and position to override animator defaults
    nw4r::lyt::Pane* capsul = this->vrButton.layout.GetPaneByName("capsul_null");
    if (capsul != nullptr) {
        capsul->alpha = 255;
        capsul->effectiveAlpha = 255;
        capsul->trans.y = 190.0f;
        capsul->trans.x = 240.0f;
        capsul->scale.x = 0.8f;
        capsul->scale.z = 0.8f;
    }
}

void ExpWFCModeSel::InitButton(ExpWFCModeSel& self) {
    self.InitControlGroup(7);

    self.region = 0x36B;  // Store region in the page class instead
    self.AddControl(5, self.ottButton, 0);
    self.ottButton.Load(UI::buttonFolder, "PULOTTButton", "PULOTTButton", 1, 0, 0);
    self.ottButton.buttonId = ottButtonId;
    self.ottButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.ottButton.SetOnSelectHandler(self.onButtonSelectHandler);
    
    self.AddControl(6, self.vrButton, 0);
    ControlLoader loader(&self.vrButton);
    loader.Load(UI::buttonFolder, "VRButton", "VRButton", nullptr);
    for (int i = 0; i < 4; ++i) {
        self.vrButton.positionAndscale[i].position.x = 240.0f;
        self.vrButton.positionAndscale[i].position.y = 190.0f;
        self.vrButton.positionAndscale[i].scale.x = 0.8f;
        self.vrButton.positionAndscale[i].scale.z = 0.8f;
    }
    self.vrButton.SetPosition(0.0f);

    Text::Info info;
    RKSYS::Mgr* rksysMgr = RKSYS::Mgr::sInstance;
    u32 vr = 0;
    if(rksysMgr->curLicenseId >= 0) {
        RKSYS::LicenseMgr& license = rksysMgr->licenses[rksysMgr->curLicenseId];
        vr = license.vr.points;
    }
    info.intToPass[0] = vr;
    self.ottButton.SetTextBoxMessage("go", BMG_RATING, &info);
}
kmCall(0x8064c294, ExpWFCModeSel::InitButton);

void ExpWFCModeSel::ClearModeContexts() {
    const u32 modeContexts[] = {
        PULSAR_MODE_OTT,
    };
    
    const u32 numContexts = sizeof(modeContexts) / sizeof(modeContexts[0]);
    for(u32 i = 0; i < numContexts; ++i) {
        u32 context = modeContexts[i];
        System::sInstance->context &= ~(1 << context);
    }
}

void ExpWFCModeSel::OnModeButtonClick(PushButton& modeButton, u32 hudSlotId) {
    const u32 id = modeButton.buttonId;
    ClearModeContexts();
    
    if (id == ottButtonId) {
        System::sInstance->netMgr.region = 0x36C;
        System::sInstance->context |= (1 << PULSAR_MODE_OTT);
    }
    else {
        System::sInstance->netMgr.region = 0x36B;
    }

    this->lastClickedButton = id;
    WFCModeSelect::OnModeButtonClick(modeButton, hudSlotId);
}

void ExpWFCModeSel::OnActivatePatch() {
    register ExpWFCModeSel* page;
    asm(mr page, r29;);
    register Pages::GlobeSearch* search;
    asm(mr search, r30;);
    const bool isHidden = search->searchType == 1 ? false : true; //make the button visible if continental was clicked
    page->ottButton.isHidden = isHidden;
    page->ottButton.manipulator.inaccessible = isHidden;

    page->nextPage = PAGE_NONE;
    PushButton* button = &page->vsButton;

    u32 bmgId = UI::BMG_RACE_WITH11P;

    if(System::sInstance->IsContext(PULSAR_MODE_OTT)) {
        page->lastClickedButton = ottButtonId;
        button = &page->ottButton;
        bmgId = UI::BMG_OTT_WW_BOTTOM;
    }
    else if(page->lastClickedButton == 2) {
        button = &page->battleButton;
        bmgId = UI::BMG_BATTLE_WITH6P;
    }

    page->bottomText.SetMessage(bmgId);
    button->SelectInitial(0);
}
kmCall(0x8064c5f0, ExpWFCModeSel::OnActivatePatch);

void ExpWFCModeSel::OnModeButtonSelect(PushButton& modeButton, u32 hudSlotId) {
    if(modeButton.buttonId == ottButtonId) {
        this->bottomText.SetMessage(BMG_OTT_WW_BOTTOM);
    } 
    else WFCModeSelect::OnModeButtonSelect(modeButton, hudSlotId);
}
} // namespace UI
} // namespace Pulsar