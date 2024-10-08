#include <modules/gui/gui.hpp>
#include <modules/hack/hack.hpp>
#include <modules/config/config.hpp>
#include <modules/debug/trace.hpp>

#include <Geode/modify/PlayLayer.hpp>

namespace eclipse::hacks::Level {

    class SmartStartPos : public hack::Hack {
        void init() override {
            auto tab = gui::MenuTab::find("Level");

            tab->addToggle("Smart StartPos", "level.smartstartpos")
                ->handleKeybinds()
                ->setDescription("Makes the start positions automatically pick the correct gamemode and speed\nOnly works in levels without direction changes");
        }

        [[nodiscard]] const char* getId() const override { return "Smart StartPos"; }
    };

    REGISTER_HACK(SmartStartPos)

    class $modify(SmartStartPosPLHook, PlayLayer) {
        struct Fields {
            std::vector<StartPosObject*> m_startPositions;
            std::vector<GameObject*> m_dualPortals, m_gamemodePortals, m_miniPortals, m_speedChanges, m_mirrorPortals;
        };

        GameObject* getClosestObject(std::vector<GameObject*>& vec, StartPosObject* startPos) {
            GameObject* closest = nullptr;

            std::sort(vec.begin(), vec.end(), [] (GameObject* a, GameObject* b) {
                return a->getPositionX() < b->getPositionX();
            });

            for (auto obj : vec) {
                if (obj->getPositionX() - 10 > startPos->getPositionX())
                    break;
                else if (obj->getPositionX() - 10 < startPos->getPositionX())
                    closest = obj;
            }

            return closest;
        }

        void setupStartPos(StartPosObject* startPos) {
            TRACE_FUNCTION();

            geode::log::debug("startpos: {}", startPos);
            LevelSettingsObject* startPosSettings = startPos->m_startSettings;
            LevelSettingsObject* levelSettings = PlayLayer::get()->m_levelSettings;
            
            geode::log::debug("startPosSettings: {}", startPosSettings);
            if (!startPosSettings) {
                geode::log::warn("startPosSettings not found");
                return;
            }

            geode::log::debug("levelSettings: {}", levelSettings);
            if (!levelSettings) {
                geode::log::warn("levelSettings not found");
                return;
            }

            startPosSettings->m_startDual = levelSettings->m_startDual;
            startPosSettings->m_startMode = levelSettings->m_startMode;
            startPosSettings->m_startMini = levelSettings->m_startMini;
            startPosSettings->m_startSpeed = levelSettings->m_startSpeed;

            GameObject* obj = getClosestObject(m_fields->m_dualPortals, startPos);
            if (obj)
                startPosSettings->m_startDual = obj->m_objectID == 286;

            obj = getClosestObject(m_fields->m_gamemodePortals, startPos);

            if (obj) {
                switch(obj->m_objectID) {
                    case 12:
                        startPosSettings->m_startMode = 0;
                        break;
                    case 13:
                        startPosSettings->m_startMode = 1;
                        break;
                    case 47:
                        startPosSettings->m_startMode = 2;
                        break;
                    case 111:
                        startPosSettings->m_startMode = 3;
                        break;
                    case 660:
                        startPosSettings->m_startMode = 4;
                        break;
                    case 745:
                        startPosSettings->m_startMode = 5;
                        break;
                    case 1331:
                        startPosSettings->m_startMode = 6;
                        break;
                    case 1933:
                        startPosSettings->m_startMode = 7;
                        break;
                }
            }

            obj = getClosestObject(m_fields->m_miniPortals, startPos);

            if (obj)
                startPosSettings->m_startMini = obj->m_objectID == 101;

            obj = getClosestObject(m_fields->m_speedChanges, startPos);
            if (obj) {
                switch(obj->m_objectID) {
                    case 200:
                        startPosSettings->m_startSpeed = Speed::Slow;
                        break;
                    case 201:
                        startPosSettings->m_startSpeed = Speed::Normal;
                        break;
                    case 202:
                        startPosSettings->m_startSpeed = Speed::Fast;
                        break;
                    case 203:
                        startPosSettings->m_startSpeed = Speed::Faster;
                        break;
                    case 1334:
                        startPosSettings->m_startSpeed = Speed::Fastest;
                        break;
                }
            }
        }

        bool init(GJGameLevel* level, bool unk1, bool unk2) {
            m_fields->m_dualPortals.clear();
            m_fields->m_gamemodePortals.clear();
            m_fields->m_miniPortals.clear();
            m_fields->m_miniPortals.clear();
            m_fields->m_speedChanges.clear();
            m_fields->m_mirrorPortals.clear();
            m_fields->m_startPositions.clear();

            return PlayLayer::init(level, unk1, unk2);
        }

        void resetLevel() {
            if (config::get<bool>("level.smartstartpos", false)) {
                TRACE_SCOPE("resetLevel() 'smart startpos'");
                for (StartPosObject* obj : m_fields->m_startPositions) {
                    TRACE_SCOPE("m_startPositions iteration");
                    setupStartPos(obj);
                }
            }

            PlayLayer::resetLevel();
        }

        void addObject(GameObject* obj) {
            PlayLayer::addObject(obj);

            switch(obj->m_objectID)
            {
                case 31:
                    m_fields->m_startPositions.push_back((StartPosObject*)obj);
                    break;
                case 12:
                case 13:
                case 47:
                case 111:
                case 660:
                case 745:
                case 1331:
                case 1933:
                    m_fields->m_gamemodePortals.push_back(obj);
                    break;
                case 45:
                case 46:
                    m_fields->m_mirrorPortals.push_back(obj);
                    break;
                case 99:
                case 101:
                    m_fields->m_miniPortals.push_back(obj);
                    break;
                case 286:
                case 287:
                    m_fields->m_dualPortals.push_back(obj);
                    break;
                case 200:
                case 201:
                case 202:
                case 203:
                case 1334:
                    m_fields->m_speedChanges.push_back(obj);
                    break;
            }
        }

    };
}
