<template>
  <div class="navbar">
    <!-- <hamburger id="hamburger-container" :is-active="sidebar.opened" class="hamburger-container" @toggleClick="toggleSideBar" />

    <breadcrumb id="breadcrumb-container" class="breadcrumb-container" v-if="!topNav"/>
    <top-nav id="topmenu-container" class="topmenu-container" v-if="topNav"/> -->
    <logo style="max-width: 100px" />
    <div v-wResize="onMenuContResize" id="navbar-left" class="navbar__left">
      <top-nav
        v-if="menuContWidth"
        id="topmenu-container"
        class="topmenu-container"
        :width="menuContWidth"
      />
    </div>

    <div class="right-menu">
      <div class="navbar-right-cluster">
        <el-dropdown
          class="language-switcher right-menu-item hover-effect"
          trigger="hover"
        >
          <div class="language-wrapper">
            <svg-icon icon-class="globel" class-name="navbar-lang-icon" />
          </div>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="changeLanguage('zh')">
              中文
            </el-dropdown-item>
            <el-dropdown-item @click.native="changeLanguage('en')">
              English
            </el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>

        <el-dropdown
          class="avatar-container right-menu-item hover-effect"
          trigger="click"
        >
          <div class="avatar-wrapper">
            <span class="navbar-user-name">{{ name }}</span>
          </div>
        <el-dropdown-menu slot="dropdown">
          <router-link to="/user/profile">
            <el-dropdown-item>{{ $t("dashboard.个人中心") }}</el-dropdown-item>
          </router-link>
          <el-dropdown-item @click.native="setting = true">
            <span>{{ $t("dashboard.布局设置") }}</span>
          </el-dropdown-item>
          <el-dropdown-item divided @click.native="logout">
            <span>{{ $t("dashboard.退出登录") }}</span>
          </el-dropdown-item>
        </el-dropdown-menu>
      </el-dropdown>
      </div>
    </div>
  </div>
</template>

<script>
import { mapGetters } from "vuex";
import Logo from "./Sidebar/Logo";
import Breadcrumb from "@/components/Breadcrumb";
import TopNav from "@/components/TopNav";
import Hamburger from "@/components/Hamburger";
import Screenfull from "@/components/Screenfull";
import SizeSelect from "@/components/SizeSelect";
import Search from "@/components/HeaderSearch";
import RuoYiGit from "@/components/RuoYi/Git";
import RuoYiDoc from "@/components/RuoYi/Doc";

export default {
  data() {
    return {
      menuContWidth: 0,
    };
  },
  components: {
    Logo,
    Breadcrumb,
    TopNav,
    Hamburger,
    Screenfull,
    SizeSelect,
    Search,
    RuoYiGit,
    RuoYiDoc,
  },
  computed: {
    ...mapGetters(["sidebar", "avatar", "device", "name"]),
    setting: {
      get() {
        return this.$store.state.settings.showSettings;
      },
      set(val) {
        this.$store.dispatch("settings/changeSetting", {
          key: "showSettings",
          value: val,
        });
      },
    },
    topNav: {
      get() {
        return this.$store.state.settings.topNav;
      },
    },
  },
  methods: {
    toggleSideBar() {
      this.$store.dispatch("app/toggleSideBar");
    },
    // 菜单容器宽度变化回调
    onMenuContResize({ width }) {
      this.menuContWidth = width;
    },
    async logout() {
      this.$confirm(
        this.$t("dashboard.确定注销并退出系统吗？"),
        this.$t("dashboard.提示"),
        {
          confirmButtonText: this.$t("dashboard.确定"),
          cancelButtonText: this.$t("dashboard.取消"),
          type: "warning",
        }
      )
        .then(() => {
          this.$store.dispatch("LogOut").then(() => {
            location.href = "/index";
          });
        })
        .catch(() => {});
    },
    changeLanguage(lang) {
      this.$i18n.locale = lang;
      localStorage.setItem("lang", lang);
      location.reload(); // 切换语言后刷新页面
    },
  },
};
</script>

<style lang="scss" scoped>
.navbar {
  height: 60px; // 修改后的高度
  overflow: hidden;
  position: relative;
  background: #fff;
  box-shadow: 0 1px 4px rgba(0, 21, 41, 0.08);
  display: flex;

  &__left {
    flex: 1;
    overflow: hidden;
    margin-left: 10px;
    margin-right: 20px;
  }

  .hamburger-container {
    line-height: 56px; // 根据新高度调整
    height: 100%;
    float: left;
    cursor: pointer;
    transition: background 0.3s;
    -webkit-tap-highlight-color: transparent;

    &:hover {
      background: rgba(0, 0, 0, 0.025);
    }
  }

  .breadcrumb-container {
    float: left;
  }

  .topmenu-container {
    display: inline-flex;
    align-items: center;
    height: 60px;
    border: none;
    font-weight: bold; // 一级菜单字体加粗
  }

  .errLog-container {
    display: inline-block;
    vertical-align: top;
  }

  .right-menu {
    flex-shrink: 0;
    margin-left: auto;
    height: 100%;
    display: flex;
    align-items: center;
    padding-right: 12px;

    &:focus {
      outline: none;
    }

    .navbar-right-cluster {
      display: flex;
      flex-direction: row;
      align-items: center;
      flex-wrap: nowrap;
      gap: 20px;
    }

    .right-menu-item {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      padding: 0 6px;
      height: 100%;
      font-size: 18px;
      color: #5a5e66;
      flex: 0 0 auto;

      &.hover-effect {
        cursor: pointer;
        transition: background 0.3s;

        &:hover {
          background: rgba(0, 0, 0, 0.025);
        }
      }
    }

    .avatar-container {
      margin: 0;
      max-width: 160px;

      .avatar-wrapper {
        margin-top: 0;
        position: relative;
        display: inline-flex;
        align-items: center;
        justify-content: flex-start;
        min-width: 48px;
        padding-left: 4px;
        white-space: nowrap;

        .navbar-user-name {
          display: inline-block;
          max-width: 140px;
          overflow: hidden;
          text-overflow: ellipsis;
          line-height: 1.2;
        }

        .user-avatar {
          cursor: pointer;
          width: 40px;
          height: 40px;
          border-radius: 10px;
        }

        .el-icon-caret-bottom {
          cursor: pointer;
          position: absolute;
          right: -20px;
          top: 25px;
          font-size: 12px;
        }
      }
    }

    .language-switcher {
      margin: 0;
      flex: 0 0 auto;

      .language-wrapper {
        cursor: pointer;
        display: inline-flex;
        align-items: center;
        justify-content: center;
        box-sizing: border-box;
        width: 40px;
        height: 40px;
        padding: 0;
      }

      /* scoped 无法作用到 SvgIcon 内部，需穿透 */
      .language-wrapper ::v-deep .navbar-lang-icon {
        display: block !important;
        margin: 0 !important;
        width: 22px !important;
        height: 22px !important;
        flex-shrink: 0;
      }
    }
  }
}
</style>
