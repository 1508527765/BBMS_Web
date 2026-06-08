<template>
  <div
    class="sidebar-logo-container"
    :class="{ collapse: collapse }"
    :style="{
      backgroundColor: 'transparent',
    }"
  >
    <transition name="sidebarLogoFade">
      <div class="sidebar-logo-link">
        <img v-if="logo" :src="logo" class="sidebar-logo" />
        <h1
          class="sidebar-title"
          :style="{
            color:
              sideTheme === 'theme-dark'
                ? variables.logoTitleColor
                : variables.logoLightTitleColor,
          }"
        >
          {{ title }}
        </h1>
      </div>
    </transition>
  </div>
</template>

<script>
import logoImg from "@/assets/logo/logo.png";
import variables from "@/assets/styles/variables.scss";
import { mapState } from "vuex";

export default {
  name: "SidebarLogo",
  props: {
    collapse: {
      type: Boolean,
      default: false,
    },
  },
  computed: {
    variables() {
      return variables;
    },
    sideTheme() {
      return this.$store.state.settings.sideTheme;
    },
    title() {
      return (process.env.VUE_APP_TITLE || "Battery Management System").replace(
        /^['"]|['"]$/g,
        ""
      );
    },
    ...mapState({
      indexPage: (state) => state.permission.indexPage,
    }),
  },
  data() {
    return {
      logo: logoImg,
    };
  },
};
</script>

<style lang="scss" scoped>
.sidebarLogoFade-enter-active {
  transition: opacity 1.5s;
}

.sidebarLogoFade-enter,
.sidebarLogoFade-leave-to {
  opacity: 0;
}

.sidebar-logo-container {
  position: relative;
  width: 200px; // 固定宽度
  height: 50px;
  line-height: 50px;
  background: transparent; // 背景色透明
  text-align: center;
  overflow: hidden;

  & .sidebar-logo-link {
    height: 100%;
    width: 100%;

    & .sidebar-logo {
      width: 32px;
      height: 32px;
      vertical-align: middle;
      margin-right: 12px;
    }

    & .sidebar-title {
      display: inline-block;
      margin: 0;
      color: #fff;
      font-weight: 600;
      line-height: 50px;
      font-size: 14px;
      font-family: Avenir, Helvetica Neue, Arial, Helvetica, sans-serif;
      vertical-align: middle;
    }
  }

  &.collapse {
    .sidebar-logo {
      margin-right: 0px;
    }
  }
}
</style>
